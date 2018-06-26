/* Copyright (c) 2015 - 2017, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software, with or without modification, must only be used with a Nordic
 *    Semiconductor ASA integrated circuit.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "highlevel.h"

#include <vector>

#include "highlevel_common.h"
#include "highlevel_batons.h"
#include "highlevel_helpers.h"

#include "utility/conversion.h"
#include "utility/errormessage.h"
#include "utility/utility.h"

#include <sstream>
#include <iostream>

#define MAX_SERIAL_NUMBERS 100
#define REPORTABLE_PROGESS 5

Nan::Persistent<v8::Function> HighLevel::constructor;
LibraryFunctionPointersType HighLevel::libraryFunctions;
bool HighLevel::loaded = false;
bool HighLevel::connectedToDevice = false;
std::string HighLevel::logMessage;
std::timed_mutex HighLevel::logMutex;
std::unique_ptr<Nan::Callback> HighLevel::jsProgressCallback;
uv_async_t *HighLevel::progressEvent = nullptr;
bool HighLevel::keepDeviceOpen = false;
Probe_handle_t HighLevel::probe;

struct ProgressData {
    std::string process;
};

ProgressData progress;

NAN_MODULE_INIT(HighLevel::Init)
{
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("nRFjprog").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    init(tpl);

    constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("nRFjprog").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(HighLevel::New)
{
    if (info.IsConstructCall())
    {
        auto obj = new HighLevel();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New(constructor);
        info.GetReturnValue().Set(cons->NewInstance());
    }
}

HighLevel::HighLevel()
{
    keepDeviceOpen = false;
}

void HighLevel::CallFunction(Nan::NAN_METHOD_ARGS_TYPE info,
                            parse_parameters_function_t parse,
                            execute_function_t execute,
                            return_function_t ret,
                            const bool hasSerialNumber)
{
    // This is a check that there exists a parse- and execute function, both of which are
    // needed to parse arguments and execute the function.
    // If this shows up in production, it is due to missing functions in the relevant
    // NAN_METHOD defining the functions.
    if (parse == nullptr ||
        execute == nullptr)
    {
        auto message = ErrorMessage::getErrorMessage(1, nrfjprog_js_err_map, std::string("One or more of the parse, or execute functions is missing for this function"));
        Nan::ThrowError(message);
        return;
    }

    logMessage.clear();

    auto argumentCount = 0;
    Baton *baton = nullptr;
    uint32_t serialNumber = 0;

    jsProgressCallback.reset();

    try
    {
        if (hasSerialNumber)
        {
            serialNumber = Convert::getNativeUint32(info[argumentCount]);
            argumentCount++;
        }

        baton = parse(info, argumentCount);

        if (baton->mayHaveProgressCallback && (argumentCount + 1) < info.Length())
        {
            v8::Local<v8::Function> callback = Convert::getCallbackFunction(info[argumentCount]);
            jsProgressCallback = std::make_unique<Nan::Callback>(callback);
            argumentCount++;
        }

        v8::Local<v8::Function> callback = Convert::getCallbackFunction(info[argumentCount]);
        baton->callback = std::make_unique<Nan::Callback>(callback);
        argumentCount++;

        if (info.Length() > argumentCount)
        {
            argumentCount = CUSTOM_ARGUMENT_PARSE_ERROR;
            std::ostringstream errorStringStream;
            errorStringStream << "Too many parameters. The function " << baton->name << " does not take " << info.Length() << " parameters.";
            throw errorStringStream.str();
        }
    }
    catch (std::string error)
    {
        if (baton != nullptr)
        {
            delete baton;
        }

        jsProgressCallback.reset();

        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);

        return;
    }

    // This is a check that there exists a returnfunction when there are more returns
    // than just err. If this shows up in production, it is due to missing return function
    if (ret == nullptr &&
        baton->returnParameterCount > 0)
    {
        auto message = ErrorMessage::getErrorMessage(1, nrfjprog_js_err_map, std::string("The return function has more than one parameter and is required for this function, but is missing"));
        Nan::ThrowError(message);
        return;
    }

    log("===============================================\n");
    log("Start of ");
    log(baton->name.c_str());
    log("\n");
    log("===============================================\n");


    baton->executeFunction = execute;
    baton->returnFunction = ret;
    baton->serialNumber = serialNumber;

    uv_queue_work(uv_default_loop(), baton->req.get(), ExecuteFunction, reinterpret_cast<uv_after_work_cb>(ReturnFunction));
}

void HighLevel::ExecuteFunction(uv_work_t *req)
{
    auto baton = static_cast<Baton *>(req->data);

    std::unique_lock<std::timed_mutex> lock (baton->executionMutex, std::defer_lock);

    if(!lock.try_lock_for(std::chrono::seconds(10))) {
        baton->result = CouldNotExecuteDueToLoad;
        return;
    }

    if (baton->mayHaveProgressCallback
        && jsProgressCallback != nullptr)
    {
        progressEvent = new uv_async_t();
        uv_async_init(uv_default_loop(), progressEvent, sendProgress);
    }

    baton->result = loadLibrary();

    if (baton->result != errorcode_t::JsSuccess)
    {
        return;
    }

    bool isOpen;
    libraryFunctions.is_dll_open(&isOpen);

    if (!isOpen)
    {
        nrfjprogdll_err_t openError = libraryFunctions.dll_open(nullptr, &HighLevel::log, &HighLevel::progressCallback);

        if (openError != SUCCESS)
        {
            baton->result = errorcode_t::CouldNotOpenDLL;
            baton->lowlevelError = openError;
            return;
        }
    }

    if (baton->serialNumber != 0
        && !keepDeviceOpen)
    {
        nrfjprogdll_err_t initError = libraryFunctions.probe_init(&probe, baton->serialNumber, nullptr);

        if (initError != SUCCESS)
        {
            baton->result = errorcode_t::CouldNotOpenDevice;
            baton->lowlevelError = initError;
            return;
        }
    }

    nrfjprogdll_err_t executeError = baton->executeFunction(baton, probe);

    if (!keepDeviceOpen)
    {
        if (baton->serialNumber != 0)
        {
            nrfjprogdll_err_t resetError = libraryFunctions.reset(probe, RESET_SYSTEM);

            if (resetError != SUCCESS)
            {
                baton->result = errorcode_t::CouldNotResetDevice;
                baton->lowlevelError = resetError;
                return;
            }

            nrfjprogdll_err_t uninitError = libraryFunctions.probe_uninit(&probe);

            if (uninitError != SUCCESS)
            {
                baton->result = errorcode_t::CouldNotCloseDevice;
                baton->lowlevelError = uninitError;
                return;
            }
        }

        libraryFunctions.dll_close();

        unloadLibrary();
    }

    if (executeError != SUCCESS)
    {
        baton->result = errorcode_t::CouldNotCallFunction;
        baton->lowlevelError = executeError;
    }

    if (progressEvent != nullptr)
    {
        auto handle = reinterpret_cast<uv_handle_t *>(progressEvent);

        uv_close(handle, [](uv_handle_t *closeHandle)
        {
            delete closeHandle;
        });

        progressEvent = nullptr;
    }
}

void HighLevel::ReturnFunction(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<Baton *>(req->data);
    std::vector<v8::Local<v8::Value> > argv;

    argv.push_back(ErrorMessage::getErrorMessage(baton->result, nrfjprog_js_err_map, baton->name, logMessage, baton->lowlevelError));

    if (baton->result != errorcode_t::JsSuccess)
    {
        for (uint32_t i = 0; i < baton->returnParameterCount; i++)
        {
            argv.push_back(Nan::Undefined());
        }
    }
    else
    {
        if (baton->returnFunction != nullptr)
        {
            std::vector<v8::Local<v8::Value> > vector = baton->returnFunction(baton);

            argv.insert(argv.end(), vector.begin(), vector.end());
        }
    }

    jsProgressCallback.reset();

    baton->callback->Call(baton->returnParameterCount + 1, argv.data());

    delete baton;
}

void HighLevel::log(const char * msg)
{
    std::unique_lock<std::timed_mutex> lock (logMutex, std::defer_lock);

    if(!lock.try_lock_for(std::chrono::seconds(10))) {
        return;
    }

    logMessage.append(msg);
}

void HighLevel::progressCallback(const char * process)
{
    if (jsProgressCallback != nullptr)
    {
        progress.process = std::string(process);

        uv_async_send(progressEvent);
    }
}

void HighLevel::sendProgress(uv_async_t *handle)
{
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[1];

    v8::Local<v8::Object> progressObj = Nan::New<v8::Object>();
    Utility::Set(progressObj, "process", Convert::toJsString(progress.process));

    argv[0] = progressObj;

    if (jsProgressCallback != nullptr)
    {
        jsProgressCallback->Call(1, argv);
    }
}

errorcode_t HighLevel::loadLibrary()
{
    if (loaded)
    {
        return errorcode_t::JsSuccess;
    }

    errorcode_t library_load_result = loadHighLevelFunctions(&libraryFunctions);
    loaded = library_load_result == errorcode_t::JsSuccess;

    return library_load_result;
}

void HighLevel::unloadLibrary()
{
    if (loaded)
    {
        loaded = false;
        releaseHighLevel();
    }
}

void HighLevel::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "getDllVersion", GetLibraryVersion); // Deprecated
    Nan::SetPrototypeMethod(tpl, "getLibraryVersion", GetLibraryVersion);
    Nan::SetPrototypeMethod(tpl, "getConnectedDevices", GetConnectedDevices);
    Nan::SetPrototypeMethod(tpl, "getSerialNumbers", GetSerialNumbers);
    Nan::SetPrototypeMethod(tpl, "getDeviceInfo", GetDeviceInfo);
    Nan::SetPrototypeMethod(tpl, "getProbeInfo", GetProbeInfo);
    Nan::SetPrototypeMethod(tpl, "getLibraryInfo", GetLibraryInfo);
    Nan::SetPrototypeMethod(tpl, "read", Read);
    Nan::SetPrototypeMethod(tpl, "readU32", ReadU32);

    Nan::SetPrototypeMethod(tpl, "program", Program);
    Nan::SetPrototypeMethod(tpl, "readToFile", ReadToFile);
    Nan::SetPrototypeMethod(tpl, "verify", Verify);
    Nan::SetPrototypeMethod(tpl, "erase", Erase);

    Nan::SetPrototypeMethod(tpl, "recover", Recover);

    Nan::SetPrototypeMethod(tpl, "write", Write);
    Nan::SetPrototypeMethod(tpl, "writeU32", WriteU32);

    Nan::SetPrototypeMethod(tpl, "open", OpenDevice);
    Nan::SetPrototypeMethod(tpl, "close", CloseDevice);
}

void HighLevel::initConsts(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAA_REV1);
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAA_REV2);
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAA_REV3);
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAB_REV3);
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAC_REV3);
    NODE_DEFINE_CONSTANT(target, NRF51802_xxAA_REV3);
    NODE_DEFINE_CONSTANT(target, NRF51801_xxAB_REV3);
    NODE_DEFINE_CONSTANT(target, NRF51_XLR1);
    NODE_DEFINE_CONSTANT(target, NRF51_XLR2);
    NODE_DEFINE_CONSTANT(target, NRF51_XLR3);
    NODE_DEFINE_CONSTANT(target, NRF51_L3);
    NODE_DEFINE_CONSTANT(target, NRF51_XLR3P);
    NODE_DEFINE_CONSTANT(target, NRF51_XLR3LC);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_ENGA);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_ENGB);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_REV1);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_REV2);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_REV1);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_REV2);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_ENGA);
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52810_xxAA_REV1);
    NODE_DEFINE_CONSTANT(target, NRF52810_xxAA_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52_FP1_ENGA);
    NODE_DEFINE_CONSTANT(target, NRF52_FP1_ENGB);
    NODE_DEFINE_CONSTANT(target, NRF52_FP1);
    NODE_DEFINE_CONSTANT(target, NRF52_FP1_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52_FP2_ENGA);

    NODE_DEFINE_CONSTANT(target, NRF51_FAMILY);
    NODE_DEFINE_CONSTANT(target, NRF52_FAMILY);
    NODE_DEFINE_CONSTANT(target, UNKNOWN_FAMILY);

    NODE_DEFINE_CONSTANT(target, ERASE_NONE);
    NODE_DEFINE_CONSTANT(target, ERASE_ALL);
    NODE_DEFINE_CONSTANT(target, ERASE_PAGES);
    NODE_DEFINE_CONSTANT(target, ERASE_PAGES_INCLUDING_UICR);

    NODE_DEFINE_CONSTANT(target, JsSuccess);
    NODE_DEFINE_CONSTANT(target, CouldNotFindJlinkDLL);
    NODE_DEFINE_CONSTANT(target, CouldNotFindJprogDLL);
    NODE_DEFINE_CONSTANT(target, CouldNotLoadDLL);
    NODE_DEFINE_CONSTANT(target, CouldNotOpenDevice);
    NODE_DEFINE_CONSTANT(target, CouldNotOpenDLL);
    NODE_DEFINE_CONSTANT(target, CouldNotConnectToDevice);
    NODE_DEFINE_CONSTANT(target, CouldNotCallFunction);
    NODE_DEFINE_CONSTANT(target, CouldNotErase);
    NODE_DEFINE_CONSTANT(target, CouldNotProgram);
    NODE_DEFINE_CONSTANT(target, CouldNotRead);
    NODE_DEFINE_CONSTANT(target, CouldNotOpenHexFile);

    NODE_DEFINE_CONSTANT(target, RESET_NONE);
    NODE_DEFINE_CONSTANT(target, RESET_SYSTEM);
    NODE_DEFINE_CONSTANT(target, RESET_DEBUG);
    NODE_DEFINE_CONSTANT(target, RESET_PIN);

    NODE_DEFINE_CONSTANT(target, ERASE_NONE);
    NODE_DEFINE_CONSTANT(target, ERASE_ALL);
    NODE_DEFINE_CONSTANT(target, ERASE_PAGES);
    NODE_DEFINE_CONSTANT(target, ERASE_PAGES_INCLUDING_UICR);

    NODE_DEFINE_CONSTANT(target, VERIFY_NONE);
    NODE_DEFINE_CONSTANT(target, VERIFY_READ);

    NODE_DEFINE_CONSTANT(target, INPUT_FORMAT_HEX_FILE);
    NODE_DEFINE_CONSTANT(target, INPUT_FORMAT_HEX_STRING);
}

NAN_METHOD(HighLevel::GetLibraryVersion)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new GetLibraryVersionBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetLibraryVersionBaton*>(b);
        return libraryFunctions.dll_get_version(&baton->major, &baton->minor, &baton->revision);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = static_cast<GetLibraryVersionBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Object> versionObj = Nan::New<v8::Object>();
        Utility::Set(versionObj, "major", Convert::toJsNumber(baton->major));
        Utility::Set(versionObj, "minor", Convert::toJsNumber(baton->minor));
        Utility::Set(versionObj, "revision", Convert::toJsNumber(baton->revision));

        returnData.push_back(versionObj);

        return returnData;
    };

    CallFunction(info, p, e, r, false);
}

NAN_METHOD(HighLevel::GetConnectedDevices)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new GetConnectedDevicesBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetConnectedDevicesBaton*>(b);
        uint32_t serialNumbers[MAX_SERIAL_NUMBERS];
        uint32_t available = 0;
        nrfjprogdll_err_t error = libraryFunctions.get_connected_probes(serialNumbers, MAX_SERIAL_NUMBERS, &available);

        if (error != SUCCESS)
        {
            return error;
        }

        for (uint32_t i = 0; i < available; i++)
        {
            Probe_handle_t getInfoProbe;
            nrfjprogdll_err_t initError = libraryFunctions.probe_init(&getInfoProbe, serialNumbers[i], nullptr);

            device_info_t device_info;
            probe_info_t probe_info;
            library_info_t library_info;

            if (initError == SUCCESS)
            {
                libraryFunctions.get_device_info(getInfoProbe, &device_info);
                libraryFunctions.get_probe_info(getInfoProbe, &probe_info);
                libraryFunctions.get_library_info(getInfoProbe, &library_info);

                libraryFunctions.probe_uninit(&getInfoProbe);
            }
            baton->probes.push_back(std::make_unique<ProbeDetails>(serialNumbers[i], device_info, probe_info, library_info));
        }

        return SUCCESS;
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = static_cast<GetConnectedDevicesBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Array> connectedDevices = Nan::New<v8::Array>();
        int i = 0;
        for (auto& element : baton->probes)
        {
            Nan::Set(connectedDevices, Convert::toJsNumber(i), element->ToJs());
            i++;
        }

        returnData.push_back(connectedDevices);

        return returnData;
    };

    CallFunction(info, p, e, r, false);
}

NAN_METHOD(HighLevel::GetSerialNumbers)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new GetSerialNumbersBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetSerialNumbersBaton*>(b);
        uint32_t serialNumbers[MAX_SERIAL_NUMBERS];
        uint32_t available = 0;
        nrfjprogdll_err_t error = libraryFunctions.get_connected_probes(serialNumbers, MAX_SERIAL_NUMBERS, &available);

        if (error != SUCCESS)
        {
            return error;
        }

        for (uint32_t i = 0; i < available; i++)
        {
            baton->serialNumbers.push_back(serialNumbers[i]);
        }

        return SUCCESS;
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = static_cast<GetSerialNumbersBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Array> serialNumbers = Nan::New<v8::Array>();
        int i = 0;
        for (auto serialNumber : baton->serialNumbers)
        {
            Nan::Set(serialNumbers, Convert::toJsNumber(i), Convert::toJsNumber(serialNumber));
            i++;
        }

        returnData.push_back(serialNumbers);

        return returnData;
    };

    CallFunction(info, p, e, r, false);
}

NAN_METHOD(HighLevel::GetProbeInfo)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new GetProbeInfoBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetProbeInfoBaton*>(b);
        return libraryFunctions.get_probe_info(probe, &baton->probeInfo);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = static_cast<GetProbeInfoBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.push_back(ProbeInfo(baton->probeInfo).ToJs());

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::GetLibraryInfo)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new GetLibraryInfoBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetLibraryInfoBaton*>(b);
        return libraryFunctions.get_library_info(probe, &baton->libraryInfo);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = static_cast<GetLibraryInfoBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.push_back(LibraryInfo(baton->libraryInfo).ToJs());

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::GetDeviceInfo)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new GetDeviceInfoBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetDeviceInfoBaton*>(b);
        return libraryFunctions.get_device_info(probe, &baton->deviceInfo);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = static_cast<GetDeviceInfoBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.push_back(DeviceInfo(baton->deviceInfo).ToJs());

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::Read)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new ReadBaton();

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->length = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<ReadBaton*>(b);
        baton->data.resize(baton->length, 0);
        return libraryFunctions.read(probe, baton->address, baton->data.data(), baton->length);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = static_cast<ReadBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.push_back(Convert::toJsValueArray(baton->data.data(), baton->length));

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::ReadU32)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new ReadU32Baton();

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<ReadU32Baton*>(b);
        return libraryFunctions.read_u32(probe, baton->address, &baton->data);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = static_cast<ReadU32Baton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.push_back(Convert::toJsNumber(baton->data));

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::Program)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new ProgramBaton();

        baton->file = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        v8::Local<v8::Object> programOptions = Convert::getJsObject(parameters[argumentCount]);
        ProgramOptions options(programOptions);
        baton->options = options.options;
        baton->inputFormat = options.inputFormat;
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<ProgramBaton*>(b);
        nrfjprogdll_err_t programResult = SUCCESS;

        FileFormatHandler file(baton->file, baton->inputFormat);

        if (!file.exists())
        {
            log(file.errormessage().c_str());
            log("\n");
            return INVALID_PARAMETER;
        }

        baton->filename = file.getFileName();

        programResult = libraryFunctions.program(probe, baton->filename.c_str(), baton->options);

        if (programResult == NOT_AVAILABLE_BECAUSE_PROTECTION &&
            baton->options.chip_erase_mode == ERASE_ALL)
        {
            const nrfjprogdll_err_t recoverResult = libraryFunctions.recover(probe);

            if (recoverResult == SUCCESS)
            {
                programResult = libraryFunctions.program(probe, baton->filename.c_str(), baton->options);
            }
            else
            {
                programResult = recoverResult;
            }
        }

        return programResult;
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::ReadToFile)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new ReadToFileBaton();

        baton->filename = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        v8::Local<v8::Object> readOptions = Convert::getJsObject(parameters[argumentCount]);
        ReadToFileOptions options(readOptions);
        baton->options = options.options;
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<ReadToFileBaton*>(b);
        return libraryFunctions.read_to_file(probe, baton->filename.c_str(), baton->options);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Verify)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new VerifyBaton();

        baton->filename = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        // There are no verify options at the moment, but there will be options
        // (like the option that the incomming content may be a string)
        v8::Local<v8::Object> verifyOptions = Convert::getJsObject(parameters[argumentCount]);
        VerifyOptions options(verifyOptions);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<VerifyBaton*>(b);
        return libraryFunctions.verify(probe, baton->filename.c_str(), VERIFY_READ);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Erase)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new EraseBaton();

        v8::Local<v8::Object> eraseOptions = Convert::getJsObject(parameters[argumentCount]);
        EraseOptions options(eraseOptions);
        baton->erase_mode = options.eraseMode;
        baton->start_address = options.startAddress;
        baton->end_address = options.endAddress;
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<EraseBaton*>(b);
        return libraryFunctions.erase(probe, baton->erase_mode, baton->start_address, baton->end_address);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Recover)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new RecoverBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        return libraryFunctions.recover(probe);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Write)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new WriteBaton();

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->data = Convert::getVectorForUint8(parameters[argumentCount]);
        baton->length = Convert::getLengthOfArray(parameters[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<WriteBaton*>(b);
        return libraryFunctions.write(probe, baton->address, baton->data.data(), baton->length);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::WriteU32)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new WriteU32Baton();

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->data = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<WriteU32Baton*>(b);
        return libraryFunctions.write_u32(probe, baton->address, baton->data);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::OpenDevice)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new OpenBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        if (keepDeviceOpen)
        {
            keepDeviceOpen = false;
            return INVALID_OPERATION; // Already opened
        }
        keepDeviceOpen = true;
        return SUCCESS;
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::CloseDevice)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new CloseBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        if (!keepDeviceOpen)
        {
            return INVALID_OPERATION; // Already closed
        }
        keepDeviceOpen = false;
        return SUCCESS;
    };

    CallFunction(info, p, e, nullptr, true);
}
