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

#include "nrfjprogjs.h"
#include "nrfjprog_common.h"
#include "nrfjprogjs_batons.h"
#include "nrfjprog_helpers.h"

#include "keilhexfile.h"
#include "dllfunc.h"

#include "utility/utility.h"
#include "utility/conversion.h"

#include <iostream>

#define MAX_SERIAL_NUMBERS 100
#define REPORTABLE_PROGESS 5

Nan::Persistent<v8::Function> nRFjprog::constructor;
DllFunctionPointersType nRFjprog::dll_function;
char nRFjprog::dll_path[COMMON_MAX_PATH] = {'\0'};
bool nRFjprog::loaded = false;
bool nRFjprog::connectedToDevice = false;
errorcodes nRFjprog::finderror = errorcodes::JsSuccess;
std::string nRFjprog::logMessage;
Nan::Callback *nRFjprog::jsProgressCallback = nullptr;
int nRFjprog::lastReportedProgress = -1;
uv_async_t *nRFjprog::progressEvent = nullptr;

struct ProgressData {
    int step;
    int steps;
    std::string process;
    int percent;
};

ProgressData progress;

NAN_MODULE_INIT(nRFjprog::Init)
{
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("nRFjprog").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    init(tpl);

    constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("nRFjprog").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(nRFjprog::New)
{
    if (info.IsConstructCall())
    {
        auto obj = new nRFjprog();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New(constructor);
        info.GetReturnValue().Set(cons->NewInstance());
    }
}

nRFjprog::nRFjprog()
{
    finderror = errorcodes::JsSuccess;
    progressEvent = new uv_async_t();
    uv_async_init(uv_default_loop(), progressEvent, sendProgress);

    NrfjprogErrorCodesType dll_find_result = OSFilesFindDll(dll_path, COMMON_MAX_PATH);

    if (dll_find_result != Success)
    {
        finderror = errorcodes::CouldNotFindJprogDLL;
    }
}

void nRFjprog::CallFunction(Nan::NAN_METHOD_ARGS_TYPE info, parse_parameters_function_t parse, execute_function_t execute, return_function_t ret, const bool hasSerialNumber)
{
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
            throw std::string("too many parameters");
        }
    }
    catch (std::string error)
    {
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);

        if (baton != nullptr)
        {
            delete baton;
        }

        if (jsProgressCallback != nullptr)
        {
            delete jsProgressCallback;
            jsProgressCallback = 0;
        }

        return;
    }

    if (ret == nullptr &&
        baton->returnParameterCount > 0)
    {
        auto message = ErrorMessage::getErrorMessage(1, std::string("The return function has more than one parameter and is required for this function, but is missing"));
        Nan::ThrowError(message);
        return;
    }

    logCallback("===============================================\n");
    logCallback("Start of ");
    logCallback(baton->name.c_str());
    logCallback("\n");
    logCallback("===============================================\n");


    baton->executeFunction = execute;
    baton->returnFunction = ret;
    baton->serialNumber = serialNumber;

    uv_queue_work(uv_default_loop(), baton->req, ExecuteFunction, reinterpret_cast<uv_after_work_cb>(ReturnFunction));
}

void nRFjprog::ExecuteFunction(uv_work_t *req)
{
    auto baton = static_cast<Baton *>(req->data);
    Probe_handle_t probe;

    baton->result = loadDll();

    if (baton->result != errorcodes::JsSuccess)
    {
        return;
    }

    bool isOpen;
    dll_function.is_dll_open(&isOpen);

    if (!isOpen)
    {
        nrfjprogdll_err_t openError = dll_function.dll_open(&nRFjprog::logCallback);

        if (openError != SUCCESS)
        {
            baton->result = errorcodes::CouldNotOpenDLL;
            baton->lowlevelError = openError;
            return;
        }
    }

    if (baton->serialNumber != 0)
    {
        nrfjprogdll_err_t initError = dll_function.probe_init(&probe, baton->serialNumber, 0, 0);

        if (initError != SUCCESS)
        {
            baton->result = errorcodes::CouldNotOpenDevice;
            baton->lowlevelError = initError;
            return;
        }
    }

    nrfjprogdll_err_t excuteError = baton->executeFunction(baton, probe);

    if (baton->serialNumber != 0)
    {

        nrfjprogdll_err_t resetError = dll_function.reset(probe);

        if (resetError != SUCCESS)
        {
            baton->result = errorcodes::CouldNotResetDevice;
            baton->lowlevelError = resetError;
            return;
        }

        nrfjprogdll_err_t uninitError = dll_function.probe_uninit(&probe);

        if (uninitError != SUCCESS)
        {
            baton->result = errorcodes::CouldNotCloseDevice;
            baton->lowlevelError = uninitError;
            return;
        }
    }

    dll_function.dll_close();

    unloadDll();

    if (excuteError != SUCCESS)
    {
        baton->result = errorcodes::CouldNotCallFunction;
        baton->lowlevelError = excuteError;
    }
}

void nRFjprog::ReturnFunction(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<Baton *>(req->data);
    //TODO: Create an arrary of correct size instead of a way to large one.
    v8::Local<v8::Value> argv[10];//baton->returnParameterCount + 1];

    if (baton->result != errorcodes::JsSuccess)
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

void nRFjprog::logCallback(const char * msg)
{
    logMessage = logMessage.append(msg);
}

bool nRFjprog::shouldProgressBeReported(const int progress)
{
    if ((progress < lastReportedProgress) ||
        (progress > (lastReportedProgress + REPORTABLE_PROGESS)) ||
        (progress >= 100))
    {
        lastReportedProgress = progress;
        return true;
    }

    return false;
}

void nRFjprog::progressCallback(uint32_t step, uint32_t total_steps, const char * process)
{
    const int percent = (int)((step * 100) / total_steps);

    const bool shouldCallback = shouldProgressBeReported(percent);

    if (jsProgressCallback != nullptr && shouldCallback)
    {
        progress.step = step;
        progress.steps = total_steps;
        progress.process = std::string(process);
        progress.percent = percent;

        uv_async_send(progressEvent);
    }
}

void nRFjprog::sendProgress(uv_async_t *handle)
{
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[1];

    v8::Local<v8::Object> progressObj = Nan::New<v8::Object>();
    Utility::Set(progressObj, "process", Convert::toJsString(progress.process));
    Utility::Set(progressObj, "step", Convert::toJsNumber(progress.step));
    Utility::Set(progressObj, "steps", Convert::toJsNumber(progress.steps));
    Utility::Set(progressObj, "percent", Convert::toJsNumber(progress.percent));

    argv[0] = progressObj;

    if (jsProgressCallback != nullptr)
    {
        jsProgressCallback->Call(1, argv);
    }
}

errorcodes nRFjprog::loadDll()
{
    if (loaded)
    {
        return errorcodes::JsSuccess;
    }

    if (finderror != errorcodes::JsSuccess)
    {
        return finderror;
    }

    NrfjprogErrorCodesType dll_load_result = DllLoad(dll_path, &dll_function);
    loaded = false;

    if (dll_load_result != Success)
    {
        return errorcodes::CouldNotLoadDLL;
    }

    loaded = true;
    return errorcodes::JsSuccess;
}

nRFjprog::~nRFjprog()
{}

void nRFjprog::unloadDll()
{
    if (loaded)
    {
        loaded = false;
        DllFree();
    }
}

void nRFjprog::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "getDllVersion", GetDllVersion);
    Nan::SetPrototypeMethod(tpl, "getConnectedDevices", GetConnectedDevices);
    Nan::SetPrototypeMethod(tpl, "getFamily", GetFamily);
    Nan::SetPrototypeMethod(tpl, "getDeviceVersion", GetDeviceVersion);
    Nan::SetPrototypeMethod(tpl, "read", Read);
    Nan::SetPrototypeMethod(tpl, "readU32", ReadU32);

    Nan::SetPrototypeMethod(tpl, "program", Program);
    Nan::SetPrototypeMethod(tpl, "readToFile", ReadToFile);
    Nan::SetPrototypeMethod(tpl, "verify", Verify);
    Nan::SetPrototypeMethod(tpl, "erase", Erase);

    Nan::SetPrototypeMethod(tpl, "recover", Recover);

    Nan::SetPrototypeMethod(tpl, "write", Write);
    Nan::SetPrototypeMethod(tpl, "writeU32", WriteU32);
}

NAN_METHOD(nRFjprog::GetDllVersion)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
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

NAN_METHOD(nRFjprog::GetConnectedDevices)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        return new GetConnectedDevicesBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetConnectedDevicesBaton*>(b);
        uint32_t serialNumbers[MAX_SERIAL_NUMBERS];
        uint32_t available = 0;
        nrfjprogdll_err_t error = dll_function.get_connected_probes(serialNumbers, MAX_SERIAL_NUMBERS, &available, 0);

        if (error != SUCCESS)
        {
            return error;
        }

        for (uint32_t i = 0; i < available; i++)
        {
            Probe_handle_t getFamilyProbe;
            nrfjprogdll_err_t initError = dll_function.probe_init(&getFamilyProbe, serialNumbers[i], 0, 0);

            device_family_t family = UNKNOWN_FAMILY;

            if (initError == SUCCESS)
            {
                dll_function.get_device_family(getFamilyProbe, &family);

                dll_function.probe_uninit(&getFamilyProbe);
            }

            baton->probes.push_back(new ProbeInfo(serialNumbers[i], family));
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

#include <sstream>
#include <iostream>
static name_map_t device_family_map = {
    NAME_MAP_ENTRY(NRF51_FAMILY),
    NAME_MAP_ENTRY(NRF52_FAMILY),
    NAME_MAP_ENTRY(UNKNOWN_FAMILY)
};

NAN_METHOD(nRFjprog::GetFamily)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new GetFamilyBaton();

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetFamilyBaton*>(b);
        nrfjprogdll_err_t err =  dll_function.get_device_family(probe, &baton->family);

        std::ostringstream errorStringStream;
        errorStringStream << Convert::valueToString(baton->family, device_family_map) << " " << baton->family << std::endl;

        return err;
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<GetFamilyBaton*>(b);
        returnType vector;

        vector.push_back(Convert::toJsNumber(baton->family));

        return vector;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(nRFjprog::GetDeviceVersion)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new GetDeviceVersionBaton();

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<GetDeviceVersionBaton*>(b);
        return dll_function.get_device_version(probe, &baton->deviceVersion);
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<GetDeviceVersionBaton*>(b);
        returnType vector;

        vector.push_back(Convert::toJsNumber(baton->deviceVersion));

        return vector;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(nRFjprog::Read)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new ReadBaton();

        baton->data = nullptr;

        baton->address = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        baton->length = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<ReadBaton*>(b);
        baton->data = new uint8_t[baton->length];
        return dll_function.read(probe, baton->address, baton->data, baton->length, 0);
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<ReadBaton*>(b);
        returnType vector;

        vector.push_back(Convert::toJsValueArray(baton->data, baton->length));

        return vector;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(nRFjprog::ReadU32)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new ReadU32Baton();

        baton->address = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<ReadU32Baton*>(b);
        return dll_function.read_u32(probe, baton->address, &baton->data, 0);
    };

    return_function_t r = [&] (Baton *b) -> returnType {
        auto baton = static_cast<ReadU32Baton*>(b);
        returnType vector;

        vector.push_back(Convert::toJsNumber(baton->data));

        return vector;
    };

    CallFunction(info, p, e, r, true);
}


NAN_METHOD(nRFjprog::Program)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new ProgramBaton();

        baton->filename = Convert::getNativeString(info[argumentCount]);
        argumentCount++;

        v8::Local<v8::Object> programOptions = Convert::getJsObject(info[argumentCount]);
        ProgramOptions options(programOptions);
        baton->options = options.options;
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<ProgramBaton*>(b);
        return dll_function.program(probe, baton->filename.c_str(), &baton->options, nRFjprog::progressCallback);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(nRFjprog::ReadToFile)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new ReadToFileBaton();

        baton->filename = Convert::getNativeString(info[argumentCount]);
        argumentCount++;

        v8::Local<v8::Object> readOptions = Convert::getJsObject(info[argumentCount]);
        ReadToFileOptions options(readOptions);
        baton->options = options.options;
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<ReadToFileBaton*>(b);
        return dll_function.read_to_file(probe, baton->filename.c_str(), &baton->options, nRFjprog::progressCallback);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(nRFjprog::Verify)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new VerifyBaton();

        baton->filename = Convert::getNativeString(info[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<VerifyBaton*>(b);
        return dll_function.verify(probe, baton->filename.c_str(), nRFjprog::progressCallback);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(nRFjprog::Erase)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new EraseBaton();

        v8::Local<v8::Object> eraseOptions = Convert::getJsObject(info[argumentCount]);
        EraseOptions options(eraseOptions);
        baton->erase_mode = options.eraseMode;
        baton->start_address = options.startAddress;
        baton->end_address = options.endAddress;
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<EraseBaton*>(b);
        return dll_function.erase(probe, baton->erase_mode, baton->start_address, baton->end_address, nRFjprog::progressCallback);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(nRFjprog::Recover)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        return new RecoverBaton();
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        return dll_function.recover(probe, nRFjprog::progressCallback);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(nRFjprog::Write)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new WriteBaton();
        baton->data = nullptr;

        baton->address = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        baton->data = Convert::getNativePointerToUint8(info[argumentCount]);
        baton->length = Convert::getLengthOfArray(info[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<WriteBaton*>(b);
        return dll_function.write(probe, baton->address, baton->data, baton->length, 0);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(nRFjprog::WriteU32)
{
    parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE info, int &argumentCount) -> Baton* {
        auto baton = new WriteU32Baton();

        baton->address = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        baton->data = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        return baton;
    };

    execute_function_t e = [&] (Baton *b, Probe_handle_t probe) -> nrfjprogdll_err_t {
        auto baton = static_cast<WriteU32Baton*>(b);
        return dll_function.write_u32(probe, baton->address, baton->data, 0);
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
        NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_ENGA);
        NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_ENGB);
        NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_REV1);
        NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_REV1);
        NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_FUTURE);
        NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_FUTURE);
        NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_ENGA);
        NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_FUTURE);

        NODE_DEFINE_CONSTANT(target, NRF51_FAMILY);
        NODE_DEFINE_CONSTANT(target, NRF52_FAMILY);
        NODE_DEFINE_CONSTANT(target, UNKNOWN_FAMILY);

        NODE_DEFINE_CONSTANT(target, ERASE_NONE);
        NODE_DEFINE_CONSTANT(target, ERASE_ALL);
        NODE_DEFINE_CONSTANT(target, ERASE_SECTOR);
        NODE_DEFINE_CONSTANT(target, ERASE_SECTOR_AND_UICR);

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
    }

    NAN_MODULE_INIT(init)
    {
        initConsts(target);
        nRFjprog::Init(target);
    }
}

NODE_MODULE(pc_nrfjprog, init);
