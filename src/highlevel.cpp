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

#include <nan.h>

#include <vector>

#include "highlevel.h"
#include "highlevel_common.h"
#include "highlevel_batons.h"
#include "highlevel_helpers.h"

#include "highlevelwrapper.h"

#include "utility/conversion.h"
#include "utility/errormessage.h"
#include "utility/utility.h"

#include <sstream>
#include <iostream>

#define MAX_SERIAL_NUMBERS 100
#define REPORTABLE_PROGESS 5

Nan::Persistent<v8::Function> HighLevel::constructor;
DllFunctionPointersType HighLevel::dll_function;
char HighLevel::dll_path[COMMON_MAX_PATH] = {'\0'};
bool HighLevel::loaded = false;
bool HighLevel::connectedToDevice = false;
errorcode_t HighLevel::finderror = errorcode_t::JsSuccess;
std::string HighLevel::logMessage;
Nan::Callback *HighLevel::jsProgressCallback = nullptr;
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
    progressEvent = new uv_async_t();
    uv_async_init(uv_default_loop(), progressEvent, sendProgress);

    finderror = OSFilesFindDll(dll_path, COMMON_MAX_PATH);

    keepDeviceOpen = false;
}

void HighLevel::CallFunction(Nan::NAN_METHOD_ARGS_TYPE info, parse_parameters_function_t parse, execute_function_t execute, return_function_t ret, const bool hasSerialNumber)
{
    // This is a check that there exists a parse- and execute function, both of which are
    // needed to parse arguments and execute the function.
    // If this shows up in production, it is due to missing functions in the relevant NAN_METHOD defining the functions.
    if (parse == nullptr ||
        execute == nullptr)
    {
        auto message = ErrorMessage::getErrorMessage(1, std::string("One or more of the parse, or execute functions is missing for this function"));
        Nan::ThrowError(message);
        return;
    }

    logMessage.clear();

    auto argumentCount = 0;
    Baton *baton = nullptr;
    uint32_t serialNumber = 0;

    if (jsProgressCallback != nullptr)
    {
        delete jsProgressCallback;
        jsProgressCallback = nullptr;
    }

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
            jsProgressCallback = new Nan::Callback(callback);
            argumentCount++;
        }

        v8::Local<v8::Function> callback = Convert::getCallbackFunction(info[argumentCount]);
        baton->callback = new Nan::Callback(callback);
        argumentCount++;

        if (info.Length() > argumentCount)
        {
            argumentCount = CUSTOM_ARGUMENT_PARSE_ERROR;
            std::ostringstream errorStringStream;
            errorStringStream << "Too many parameters. The function " << baton->name << " do not take " << info.Length() << " parameters.";
            throw errorStringStream.str();
        }
    }
    catch (std::string error)
    {
        if (baton != nullptr)
        {
            delete baton;
        }

        if (jsProgressCallback != nullptr)
        {
            delete jsProgressCallback;
            jsProgressCallback = nullptr;
        }

        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);

        return;
    }

    // This is a check that there exists a returnfunction when there are more returns
    // than just err. If this shows up in production, it is due to missing return function
    if (ret == nullptr &&
        baton->returnParameterCount > 0)
    {
        auto message = ErrorMessage::getErrorMessage(1, std::string("The return function has more than one parameter and is required for this function, but is missing"));
        Nan::ThrowError(message);
        return;
    }

    log("===============================================\n");
    log("Start of ");
    log(baton->name);
    log("\n");
    log("===============================================\n");


    baton->executeFunction = execute;
    baton->returnFunction = ret;
    baton->serialNumber = serialNumber;

    uv_queue_work(uv_default_loop(), baton->req, ExecuteFunction, reinterpret_cast<uv_after_work_cb>(ReturnFunction));
}

void HighLevel::ExecuteFunction(uv_work_t *req)
{
    auto baton = static_cast<Baton *>(req->data);

    baton->result = loadDll();

    if (baton->result != errorcode_t::JsSuccess)
    {
        return;
    }

    bool isOpen;
    dll_function.is_dll_open(&isOpen);

    if (!isOpen)
    {
        nrfjprogdll_err_t openError = dll_function.dll_open(nullptr, &HighLevel::logCallback, &HighLevel::progressCallback);

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
        nrfjprogdll_err_t initError = dll_function.probe_init(&probe, baton->serialNumber, nullptr);

        if (initError != SUCCESS)
        {
            baton->result = errorcode_t::CouldNotOpenDevice;
            baton->lowlevelError = initError;
            return;
        }
    }

    nrfjprogdll_err_t excuteError = baton->executeFunction(baton, probe);

    if (!keepDeviceOpen)
    {
        if (baton->serialNumber != 0)
        {
            nrfjprogdll_err_t resetError = dll_function.reset(probe, RESET_SYSTEM);

            if (resetError != SUCCESS)
            {
                baton->result = errorcode_t::CouldNotResetDevice;
                baton->lowlevelError = resetError;
                return;
            }

            nrfjprogdll_err_t uninitError = dll_function.probe_uninit(&probe);

            if (uninitError != SUCCESS)
            {
                baton->result = errorcode_t::CouldNotCloseDevice;
                baton->lowlevelError = uninitError;
                return;
            }
        }

        dll_function.dll_close();

        unloadDll();
    }

    if (excuteError != SUCCESS)
    {
        baton->result = errorcode_t::CouldNotCallFunction;
        baton->lowlevelError = excuteError;
    }
}

void HighLevel::ReturnFunction(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<Baton *>(req->data);
    //TODO: Create an arrary of correct size instead of a way to large one.
    v8::Local<v8::Value> argv[10];//baton->returnParameterCount + 1];

    if (baton->result != errorcode_t::JsSuccess)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, baton->name, logMessage, baton->lowlevelError);

        for (uint32_t i = 0; i < baton->returnParameterCount; i++)
        {
            argv[i + 1] = Nan::Undefined();
        }
    }
    else
    {
        argv[0] = Nan::Undefined();

        if (baton->returnFunction != nullptr)
        {
            std::vector<v8::Local<v8::Value> > vector = baton->returnFunction(baton);

            for (uint32_t i = 0; i < vector.size(); ++i)
            {
                argv[i + 1] = vector[i];
            }
        }
    }

    if (jsProgressCallback != nullptr)
    {
        delete jsProgressCallback;
        jsProgressCallback = nullptr;
    }

    baton->callback->Call(baton->returnParameterCount + 1, argv);

    delete baton;
}

void HighLevel::logCallback(const char * msg)
{
    log(msg);
}

void HighLevel::log(std::string msg)
{
    logMessage = logMessage.append(msg);
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

errorcode_t HighLevel::loadDll()
{
    if (loaded)
    {
        return errorcode_t::JsSuccess;
    }

    if (finderror != errorcode_t::JsSuccess)
    {
        return finderror;
    }

    errorcode_t dll_load_result = loadFunctions(dll_path, &dll_function);
    loaded = dll_load_result == errorcode_t::JsSuccess;

    return dll_load_result;
}

HighLevel::~HighLevel()
{}

void HighLevel::unloadDll()
{
    if (loaded)
    {
        loaded = false;
        release();
    }
}

void HighLevel::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "getDllVersion", GetDllVersion);
    Nan::SetPrototypeMethod(tpl, "getConnectedDevices", GetConnectedDevices);
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

NAN_METHOD(HighLevel::GetDllVersion)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new GetDllVersionBaton();

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetDllVersionBaton*>(b);
        return dll_function.dll_get_version(&baton->major, &baton->minor, &baton->revision);
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<GetDllVersionBaton*>(b);
        returnType vector;

        v8::Local<v8::Object> versionObj = Nan::New<v8::Object>();
        Utility::Set(versionObj, "major", Convert::toJsNumber(baton->major));
        Utility::Set(versionObj, "minor", Convert::toJsNumber(baton->minor));
        Utility::Set(versionObj, "revision", Convert::toJsNumber(baton->revision));

        vector.push_back(versionObj);

        return vector;
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
        nrfjprogdll_err_t error = dll_function.get_connected_probes(serialNumbers, MAX_SERIAL_NUMBERS, &available);

        if (error != SUCCESS)
        {
            return error;
        }

        for (uint32_t i = 0; i < available; i++)
        {
            Probe_handle_t getInfoProbe;
            nrfjprogdll_err_t initError = dll_function.probe_init(&getInfoProbe, serialNumbers[i], nullptr);

            device_info_t device_info;
            probe_info_t probe_info;
            library_info_t library_info;

            if (initError == SUCCESS)
            {
                dll_function.get_device_info(getInfoProbe, &device_info);
                dll_function.get_probe_info(getInfoProbe, &probe_info);
                dll_function.get_library_info(getInfoProbe, &library_info);

                dll_function.probe_uninit(&getInfoProbe);
            }

            baton->probes.push_back(new ProbeDetails(serialNumbers[i], device_info, probe_info, library_info));
        }

        return SUCCESS;
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<GetConnectedDevicesBaton*>(b);
        returnType vector;

        v8::Local<v8::Array> connectedDevices = Nan::New<v8::Array>();
        for (uint32_t i = 0; i < baton->probes.size(); ++i)
        {
            Nan::Set(connectedDevices, Convert::toJsNumber(i), baton->probes[i]->ToJs());
        }

        vector.push_back(connectedDevices);

        return vector;
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
        return dll_function.get_probe_info(probe, &baton->probeInfo);
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<GetProbeInfoBaton*>(b);
        returnType vector;

        vector.push_back(ProbeInfo(baton->probeInfo).ToJs());

        return vector;
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
        return dll_function.get_library_info(probe, &baton->libraryInfo);
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<GetLibraryInfoBaton*>(b);
        returnType vector;

        vector.push_back(LibraryInfo(baton->libraryInfo).ToJs());

        return vector;
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
        return dll_function.get_device_info(probe, &baton->deviceInfo);
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<GetDeviceInfoBaton*>(b);
        returnType vector;

        vector.push_back(DeviceInfo(baton->deviceInfo).ToJs());

        return vector;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::Read)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new ReadBaton();

        baton->data = nullptr;

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->length = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<ReadBaton*>(b);
        baton->data = new uint8_t[baton->length];
        return dll_function.read(probe, baton->address, baton->data, baton->length);
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<ReadBaton*>(b);
        returnType vector;

        vector.push_back(Convert::toJsValueArray(baton->data, baton->length));

        return vector;
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
        return dll_function.read_u32(probe, baton->address, &baton->data);
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<ReadU32Baton*>(b);
        returnType vector;

        vector.push_back(Convert::toJsNumber(baton->data));

        return vector;
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
            log(file.errormessage());
            log("\n");
            return INVALID_PARAMETER;
        }

        baton->filename = file.getFileName();

        programResult = dll_function.program(probe, baton->filename.c_str(), baton->options);

        if (programResult == NOT_AVAILABLE_BECAUSE_PROTECTION &&
            baton->options.chip_erase_mode == ERASE_ALL)
        {
            const nrfjprogdll_err_t recoverResult = dll_function.recover(probe);

            if (recoverResult == SUCCESS)
            {
                programResult = dll_function.program(probe, baton->filename.c_str(), baton->options);
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
        return dll_function.read_to_file(probe, baton->filename.c_str(), baton->options);
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
        return dll_function.verify(probe, baton->filename.c_str(), VERIFY_READ);
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
        return dll_function.erase(probe, baton->erase_mode, baton->start_address, baton->end_address);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Recover)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new RecoverBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        return dll_function.recover(probe);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Write)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        auto baton = new WriteBaton();
        baton->data = nullptr;

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->data = Convert::getNativePointerToUint8(parameters[argumentCount]);
        baton->length = Convert::getLengthOfArray(parameters[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<WriteBaton*>(b);
        return dll_function.write(probe, baton->address, baton->data, baton->length);
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
        return dll_function.write_u32(probe, baton->address, baton->data);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::OpenDevice)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> Baton* {
        return new OpenBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
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
        keepDeviceOpen = false;
        return SUCCESS;
    };

    CallFunction(info, p, e, nullptr, true);
}
extern "C" {
    void initConsts(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
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

    NAN_MODULE_INIT(init)
    {
        initConsts(target);
        HighLevel::Init(target);
    }
}

NODE_MODULE(pc_nrfjprog, init);
