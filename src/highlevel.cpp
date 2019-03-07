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
#include <mutex>
#include <sstream>
#include <iostream>

#include "highlevel_common.h"
#include "highlevel_batons.h"
#include "highlevel_helpers.h"

#include "utility/conversion.h"
#include "utility/errormessage.h"
#include "utility/utility.h"

#define MAX_SERIAL_NUMBERS 100
#define REPORTABLE_PROGESS 5

struct HighLevelStaticPrivate
{
    LibraryFunctionPointersType libraryFunctions{};
    bool loaded{false};
    bool keepDeviceOpen{false};
    Probe_handle_t probe{};
    std::string logMessage;
    std::timed_mutex logMutex;
    std::unique_ptr<Nan::Callback> jsProgressCallback;
    std::unique_ptr<uv_async_t> progressEvent;
    std::string progressProcess;

    static inline Nan::Persistent<v8::Function> & constructor() {
        static Nan::Persistent<v8::Function> my_constructor;
        return my_constructor;
    }
};

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static HighLevelStaticPrivate highlvlStatic;

NAN_MODULE_INIT(HighLevel::Init)
{
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("nRFjprog").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    init(tpl);

    HighLevelStaticPrivate::constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("nRFjprog").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(HighLevel::New)
{
    if (info.IsConstructCall())
    {
        (new HighLevel())->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        const int argc = 1;
        v8::Local<v8::Value> argv[argc] = { info[0] };
        v8::Local<v8::Function> cons = Nan::New(HighLevelStaticPrivate::constructor());
        info.GetReturnValue().Set(Nan::NewInstance(cons, argc, static_cast< v8::Local<v8::Value>* >(argv)).ToLocalChecked());
    }
}

HighLevel::HighLevel()
{
    resetLog();
}

void HighLevel::CallFunction(Nan::NAN_METHOD_ARGS_TYPE info,
                            const parse_parameters_function_t& parse,
                            const execute_function_t& execute,
                            const return_function_t& ret,
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

    resetLog();

    auto argumentCount = 0;
    std::unique_ptr<Baton> baton;
    uint32_t serialNumber = 0;

    highlvlStatic.jsProgressCallback.reset();

    try
    {
        if (hasSerialNumber)
        {
            serialNumber = Convert::getNativeUint32(info[argumentCount]);
            argumentCount++;
        }

        baton.reset(parse(info, argumentCount));

        if (baton->mayHaveProgressCallback && (argumentCount + 1) < info.Length())
        {
            v8::Local<v8::Function> callback = Convert::getCallbackFunction(info[argumentCount]);
            highlvlStatic.jsProgressCallback = std::make_unique<Nan::Callback>(callback);
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
            throw std::runtime_error(errorStringStream.str());
        }
    }
    catch (const std::runtime_error& error)
    {
        baton = nullptr;

        highlvlStatic.jsProgressCallback.reset();

        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error.what());
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

    baton->executeFunction = execute;
    baton->returnFunction = ret;
    baton->serialNumber = serialNumber;

    uv_queue_work(uv_default_loop(), baton->req.get(), ExecuteFunction, reinterpret_cast<uv_after_work_cb>(ReturnFunction));

    (void) baton.release();
}

void HighLevel::ExecuteFunction(uv_work_t *req)
{
    auto baton = static_cast<Baton *>(req->data);

    std::unique_lock<std::timed_mutex> lock (Baton::executionMutex, std::defer_lock);

    if(!lock.try_lock_for (std::chrono::seconds(10))) {
        baton->result = CouldNotExecuteDueToLoad;
        return;
    }

    if (baton->mayHaveProgressCallback
        && highlvlStatic.jsProgressCallback)
    {
        highlvlStatic.progressEvent = std::make_unique<uv_async_t>();
        uv_async_init(uv_default_loop(), highlvlStatic.progressEvent.get(), sendProgress);
    }

    baton->result = loadLibrary();

    if (baton->result != errorcode_t::JsSuccess)
    {
        return;
    }

    bool isOpen;
    highlvlStatic.libraryFunctions.is_dll_open(&isOpen);

    if (!isOpen)
    {
        nrfjprogdll_err_t openError = highlvlStatic.libraryFunctions.dll_open(nullptr, &HighLevel::log, &HighLevel::progressCallback);

        if (openError != SUCCESS)
        {
            baton->result = errorcode_t::CouldNotOpenDLL;
            baton->lowlevelError = openError;
            return;
        }
    }

    if (baton->serialNumber != 0
        && !highlvlStatic.keepDeviceOpen)
    {
        nrfjprogdll_err_t initError = highlvlStatic.libraryFunctions.probe_init(&highlvlStatic.probe, baton->serialNumber, nullptr);

        if (initError != SUCCESS)
        {
            baton->result = errorcode_t::CouldNotOpenDevice;
            baton->lowlevelError = initError;
            return;
        }
    }

    nrfjprogdll_err_t executeError = baton->executeFunction(baton, highlvlStatic.probe);

    if (!highlvlStatic.keepDeviceOpen)
    {
        if (baton->serialNumber != 0)
        {
            nrfjprogdll_err_t resetError = highlvlStatic.libraryFunctions.reset(highlvlStatic.probe, RESET_SYSTEM);

            if (resetError != SUCCESS)
            {
                baton->result = errorcode_t::CouldNotResetDevice;
                baton->lowlevelError = resetError;
                return;
            }

            nrfjprogdll_err_t uninitError = highlvlStatic.libraryFunctions.probe_uninit(&highlvlStatic.probe);

            if (uninitError != SUCCESS)
            {
                baton->result = errorcode_t::CouldNotCloseDevice;
                baton->lowlevelError = uninitError;
                return;
            }
        }

        highlvlStatic.libraryFunctions.dll_close();

        unloadLibrary();
    }

    if (executeError != SUCCESS)
    {
        baton->result = errorcode_t::CouldNotCallFunction;
        baton->lowlevelError = executeError;
    }

    if (highlvlStatic.progressEvent)
    {
        auto handle = reinterpret_cast<uv_handle_t *>(highlvlStatic.progressEvent.get());

        uv_close(handle, [](uv_handle_t * /*closeHandle*/)
        {
            highlvlStatic.progressEvent = nullptr;
        });
    }
}

void HighLevel::ReturnFunction(uv_work_t *req)
{
    Nan::HandleScope scope;

    std::unique_ptr<Baton> baton(static_cast<Baton*>(req->data));
    std::vector<v8::Local<v8::Value> > argv;

    std::string msg;

    {
        std::unique_lock<std::timed_mutex> lock (highlvlStatic.logMutex, std::defer_lock);

        if(lock.try_lock_for (std::chrono::seconds(10))) {
            msg = highlvlStatic.logMessage;
        }
    }

    argv.emplace_back(ErrorMessage::getErrorMessage(baton->result, nrfjprog_js_err_map, baton->name, msg, baton->lowlevelError));

    if (baton->result != errorcode_t::JsSuccess)
    {
        for (uint32_t i = 0; i < baton->returnParameterCount; i++)
        {
            argv.emplace_back(Nan::Undefined());
        }
    }
    else
    {
        if (baton->returnFunction != nullptr)
        {
            std::vector<v8::Local<v8::Value> > vector = baton->returnFunction(baton.get());

            argv.insert(argv.end(), vector.begin(), vector.end());
        }
    }

    highlvlStatic.jsProgressCallback.reset();

    Nan::AsyncResource resource("pc-nrfjprog-js:callback");
    baton->callback->Call(baton->returnParameterCount + 1, argv.data(), &resource);
}


void HighLevel::log(const char *msg)
{
    log(std::string(msg));
}

void HighLevel::log(const std::string& msg)
{
    std::unique_lock<std::timed_mutex> lock (highlvlStatic.logMutex, std::defer_lock);

    if(!lock.try_lock_for (std::chrono::seconds(10))) {
        return;
    }

    highlvlStatic.logMessage.append(msg);
}

void HighLevel::resetLog()
{
    std::unique_lock<std::timed_mutex> lock (highlvlStatic.logMutex, std::defer_lock);

    if(!lock.try_lock_for (std::chrono::seconds(10))) {
        return;
    }

    highlvlStatic.logMessage.clear();
}

void HighLevel::progressCallback(const char * process)
{
    if (highlvlStatic.jsProgressCallback)
    {
        highlvlStatic.progressProcess = std::string(process);

        uv_async_send(highlvlStatic.progressEvent.get());
    }
}

void HighLevel::sendProgress(uv_async_t * /*handle*/)
{
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[1];

    v8::Local<v8::Object> progressObj = Nan::New<v8::Object>();
    Utility::Set(progressObj, "process", Convert::toJsString(highlvlStatic.progressProcess));

    argv[0] = progressObj;

    if (highlvlStatic.jsProgressCallback)
    {
        Nan::AsyncResource resource("pc-nrfjprog-js:callback");
        highlvlStatic.jsProgressCallback->Call(1, static_cast< v8::Local<v8::Value>* >(argv), &resource);
    }
}

errorcode_t HighLevel::loadLibrary()
{
    if (highlvlStatic.loaded)
    {
        return errorcode_t::JsSuccess;
    }

    errorcode_t library_load_result = loadHighLevelFunctions(&highlvlStatic.libraryFunctions);
    highlvlStatic.loaded = library_load_result == errorcode_t::JsSuccess;

    return library_load_result;
}

void HighLevel::unloadLibrary()
{
    if (highlvlStatic.loaded)
    {
        highlvlStatic.loaded = false;
        releaseHighLevel();
    }
}

void HighLevel::init(v8::Local<v8::FunctionTemplate> target)
{
    Nan::SetPrototypeMethod(target, "getDllVersion", GetLibraryVersion); // Deprecated
    Nan::SetPrototypeMethod(target, "getLibraryVersion", GetLibraryVersion);
    Nan::SetPrototypeMethod(target, "getConnectedDevices", GetConnectedDevices);
    Nan::SetPrototypeMethod(target, "getSerialNumbers", GetSerialNumbers);
    Nan::SetPrototypeMethod(target, "getDeviceInfo", GetDeviceInfo);
    Nan::SetPrototypeMethod(target, "getProbeInfo", GetProbeInfo);
    Nan::SetPrototypeMethod(target, "getLibraryInfo", GetLibraryInfo);
    Nan::SetPrototypeMethod(target, "read", Read);
    Nan::SetPrototypeMethod(target, "readU32", ReadU32);

    Nan::SetPrototypeMethod(target, "program", Program);
    Nan::SetPrototypeMethod(target, "readToFile", ReadToFile);
    Nan::SetPrototypeMethod(target, "verify", Verify);
    Nan::SetPrototypeMethod(target, "erase", Erase);

    Nan::SetPrototypeMethod(target, "recover", Recover);

    Nan::SetPrototypeMethod(target, "write", Write);
    Nan::SetPrototypeMethod(target, "writeU32", WriteU32);

    Nan::SetPrototypeMethod(target, "open", OpenDevice);
    Nan::SetPrototypeMethod(target, "close", CloseDevice);
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
    NODE_DEFINE_CONSTANT(target, NRF52810_xxAA_REV1);
    NODE_DEFINE_CONSTANT(target, NRF52810_xxAA_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_ENGA);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_ENGB);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_REV1);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_REV2);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_REV1);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_REV2);
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_ENGA);
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_ENGB);
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_REV1);
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52_FP1_ENGA);
    NODE_DEFINE_CONSTANT(target, NRF52_FP1_ENGB);
    NODE_DEFINE_CONSTANT(target, NRF52_FP1);
    NODE_DEFINE_CONSTANT(target, NRF52_FP1_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF52_FP2_ENGA);
    NODE_DEFINE_CONSTANT(target, NRF52_FP2_FUTURE);
    NODE_DEFINE_CONSTANT(target, NRF9160_xxAA_FP1);
    NODE_DEFINE_CONSTANT(target, NRF9160_xxAA_FUTURE);

    NODE_DEFINE_CONSTANT(target, NRF51_FAMILY);
    NODE_DEFINE_CONSTANT(target, NRF52_FAMILY);
    NODE_DEFINE_CONSTANT(target, NRF91_FAMILY);
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
        auto baton = dynamic_cast<GetLibraryVersionBaton*>(b);
        return highlvlStatic.libraryFunctions.dll_get_version(&baton->major, &baton->minor, &baton->revision);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<GetLibraryVersionBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Object> versionObj = Nan::New<v8::Object>();
        Utility::Set(versionObj, "major", Convert::toJsNumber(baton->major));
        Utility::Set(versionObj, "minor", Convert::toJsNumber(baton->minor));
        Utility::Set(versionObj, "revision", Convert::toJsNumber(baton->revision));

        returnData.emplace_back(versionObj);

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
        auto baton = dynamic_cast<GetConnectedDevicesBaton*>(b);
        uint32_t serialNumbers[MAX_SERIAL_NUMBERS];
        uint32_t available = 0;
        nrfjprogdll_err_t error = highlvlStatic.libraryFunctions.get_connected_probes(static_cast<uint32_t*>(serialNumbers), MAX_SERIAL_NUMBERS, &available);

        if (error != SUCCESS)
        {
            return error;
        }

        for (uint32_t i = 0; i < available; i++)
        {
            Probe_handle_t getInfoProbe;
            nrfjprogdll_err_t initError = highlvlStatic.libraryFunctions.probe_init(&getInfoProbe, serialNumbers[i], nullptr);

            device_info_t device_info;
            probe_info_t probe_info;
            library_info_t library_info;

            if (initError == SUCCESS)
            {
                highlvlStatic.libraryFunctions.get_device_info(getInfoProbe, &device_info);
                highlvlStatic.libraryFunctions.get_probe_info(getInfoProbe, &probe_info);
                highlvlStatic.libraryFunctions.get_library_info(getInfoProbe, &library_info);

                highlvlStatic.libraryFunctions.probe_uninit(&getInfoProbe);
            }
            baton->probes.emplace_back(std::make_unique<ProbeDetails>(serialNumbers[i], device_info, probe_info, library_info));
        }

        return SUCCESS;
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<GetConnectedDevicesBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Array> connectedDevices = Nan::New<v8::Array>();
        int i = 0;
        for (auto& element : baton->probes)
        {
            Nan::Set(connectedDevices, Convert::toJsNumber(i), element->ToJs());
            i++;
        }

        returnData.emplace_back(connectedDevices);

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
        auto baton = dynamic_cast<GetSerialNumbersBaton*>(b);
        uint32_t serialNumbers[MAX_SERIAL_NUMBERS];
        uint32_t available = 0;
        nrfjprogdll_err_t error = highlvlStatic.libraryFunctions.get_connected_probes(static_cast<uint32_t*>(serialNumbers), MAX_SERIAL_NUMBERS, &available);

        if (error != SUCCESS)
        {
            return error;
        }

        for (uint32_t i = 0; i < available; ++i)
        {
            baton->serialNumbers.emplace_back(serialNumbers[i]);
        }

        return SUCCESS;
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<GetSerialNumbersBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Array> serialNumbers = Nan::New<v8::Array>();
        int i = 0;
        for (auto serialNumber : baton->serialNumbers)
        {
            Nan::Set(serialNumbers, Convert::toJsNumber(i), Convert::toJsNumber(serialNumber));
            ++i;
        }

        returnData.emplace_back(serialNumbers);

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
        auto baton = dynamic_cast<GetProbeInfoBaton*>(b);
        return highlvlStatic.libraryFunctions.get_probe_info(probe, &baton->probeInfo);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<GetProbeInfoBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(ProbeInfo(baton->probeInfo).ToJs());

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
        auto baton = dynamic_cast<GetLibraryInfoBaton*>(b);
        return highlvlStatic.libraryFunctions.get_library_info(probe, &baton->libraryInfo);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<GetLibraryInfoBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(LibraryInfo(baton->libraryInfo).ToJs());

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
        auto baton = dynamic_cast<GetDeviceInfoBaton*>(b);
        return highlvlStatic.libraryFunctions.get_device_info(probe, &baton->deviceInfo);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<GetDeviceInfoBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(DeviceInfo(baton->deviceInfo).ToJs());

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::Read)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        std::unique_ptr<ReadBaton> baton(new ReadBaton());

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->length = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<ReadBaton*>(b);
        baton->data.resize(baton->length, 0);
        return highlvlStatic.libraryFunctions.read(probe, baton->address, baton->data.data(), baton->length);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<ReadBaton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(Convert::toJsValueArray(baton->data.data(), baton->length));

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::ReadU32)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        std::unique_ptr<ReadU32Baton> baton(new ReadU32Baton());

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<ReadU32Baton*>(b);
        return highlvlStatic.libraryFunctions.read_u32(probe, baton->address, &baton->data);
    };

    return_function_t r = [&] (Baton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<ReadU32Baton*>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(Convert::toJsNumber(baton->data));

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::Program)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        std::unique_ptr<ProgramBaton> baton(new ProgramBaton());

        baton->file = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        v8::Local<v8::Object> programOptions = Convert::getJsObject(parameters[argumentCount]);
        ProgramOptions options(programOptions);
        baton->options = options.options;
        baton->inputFormat = options.inputFormat;
        argumentCount++;

        return baton.release();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<ProgramBaton*>(b);
        nrfjprogdll_err_t programResult = SUCCESS;

        FileFormatHandler file(baton->file, baton->inputFormat);

        if (!file.exists())
        {
            log(file.errormessage().c_str());
            log("\n");
            return INVALID_PARAMETER;
        }

        baton->filename = file.getFileName();

        programResult = highlvlStatic.libraryFunctions.program(probe, baton->filename.c_str(), baton->options);

        if (programResult == NOT_AVAILABLE_BECAUSE_PROTECTION &&
            baton->options.chip_erase_mode == ERASE_ALL)
        {
            const nrfjprogdll_err_t recoverResult = highlvlStatic.libraryFunctions.recover(probe);

            if (recoverResult == SUCCESS)
            {
                programResult = highlvlStatic.libraryFunctions.program(probe, baton->filename.c_str(), baton->options);
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
        std::unique_ptr<ReadToFileBaton> baton(new ReadToFileBaton());

        baton->filename = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        v8::Local<v8::Object> readOptions = Convert::getJsObject(parameters[argumentCount]);
        ReadToFileOptions options(readOptions);
        baton->options = options.options;
        argumentCount++;

        return baton.release();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<ReadToFileBaton*>(b);
        return highlvlStatic.libraryFunctions.read_to_file(probe, baton->filename.c_str(), baton->options);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Verify)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        std::unique_ptr<VerifyBaton> baton(new VerifyBaton());

        baton->filename = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        // There are no verify options at the moment, but there will be options
        // (like the option that the incomming content may be a string)
        v8::Local<v8::Object> verifyOptions = Convert::getJsObject(parameters[argumentCount]);
        VerifyOptions options(verifyOptions);
        argumentCount++;

        return baton.release();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<VerifyBaton*>(b);
        return highlvlStatic.libraryFunctions.verify(probe, baton->filename.c_str(), VERIFY_READ);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Erase)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        std::unique_ptr<EraseBaton> baton(new EraseBaton());

        v8::Local<v8::Object> eraseOptions = Convert::getJsObject(parameters[argumentCount]);
        EraseOptions options(eraseOptions);
        baton->erase_mode = options.eraseMode;
        baton->start_address = options.startAddress;
        baton->end_address = options.endAddress;
        argumentCount++;

        return baton.release();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<EraseBaton*>(b);
        return highlvlStatic.libraryFunctions.erase(probe, baton->erase_mode, baton->start_address, baton->end_address);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Recover)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new RecoverBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        return highlvlStatic.libraryFunctions.recover(probe);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Write)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        std::unique_ptr<WriteBaton> baton(new WriteBaton());

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->data = Convert::getVectorForUint8(parameters[argumentCount]);
        baton->length = Convert::getLengthOfArray(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<WriteBaton*>(b);
        return highlvlStatic.libraryFunctions.write(probe, baton->address, baton->data.data(), baton->length);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::WriteU32)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        std::unique_ptr<WriteU32Baton> baton(new WriteU32Baton());

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->data = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<WriteU32Baton*>(b);
        return highlvlStatic.libraryFunctions.write_u32(probe, baton->address, baton->data);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::OpenDevice)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new OpenBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        if (highlvlStatic.keepDeviceOpen)
        {
            highlvlStatic.keepDeviceOpen = false;
            return INVALID_OPERATION; // Already opened
        }
        highlvlStatic.keepDeviceOpen = true;
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
        highlvlStatic.keepDeviceOpen = false;
        return SUCCESS;
    };

    CallFunction(info, p, e, nullptr, true);
}
