/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in or with a processor manufactured by Nordic
 *   Semiconductor ASA, or in or with a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *   5. Any software provided in binary or object form under this license must not be
 *   reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <nan.h>

#include <vector>

#include "nrfjprogjs.h"
#include "nrfjprog_common.h"
#include "nrfjprogjs_batons.h"

#include "keilhexfile.h"
#include "dllfunc.h"

#include <iostream>

#define OPEN_BEFORE_CALL() do { \
    baton->result = loadDll();\
\
    if (baton->result != errorcodes::JsSuccess) \
    { \
        return; \
    } \
\
    bool isOpen; \
    dll_function.is_dll_open(&isOpen); \
\
    if (!isOpen) \
    { \
        nrfjprogdll_err_t openError = dll_function.dll_open(&DebugProbe::logCallback); \
\
        if (openError != SUCCESS) \
        { \
            baton->result = errorcodes::CouldNotOpenDLL; \
            return; \
        } \
    } \
} while(0);

#define EXIT_AFTER_CALL() do { \
    closeBeforeExit();\
} while(0);

#define INIT_PROBE(probe, serialnumber) do { \
    nrfjprogdll_err_t initError = dll_function.probe_init(&probe, serialnumber, 0, 0); \
\
    if (initError != SUCCESS) \
    { \
        baton->result = errorcodes::CouldNotOpenDevice; \
        return; \
    } \
} while(0);

#define UNINIT_PROBE(probe) do { \
    nrfjprogdll_err_t uninitError = dll_function.probe_uninit(&probe); \
\
    if (uninitError != SUCCESS) \
    { \
        baton->result = errorcodes::CouldNotOpenDevice; \
        return; \
    } \
} while(0);

Nan::Persistent<v8::Function> DebugProbe::constructor;
DllFunctionPointersType DebugProbe::dll_function;
char DebugProbe::dll_path[COMMON_MAX_PATH] = {'\0'};
char DebugProbe::jlink_path[COMMON_MAX_PATH] = {'\0'};
bool DebugProbe::loaded = false;
bool DebugProbe::connectedToDevice = false;
errorcodes DebugProbe::finderror = errorcodes::JsSuccess;
uint32_t DebugProbe::emulatorSpeed = 1000;
std::string DebugProbe::logMessage;

v8::Local<v8::Object> ProbeInfo::ToJs()
{
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Object> obj = Nan::New<v8::Object>();

    Utility::Set(obj, "serialNumber", ConversionUtility::toJsNumber(serial_number));
    Utility::Set(obj, "family", ConversionUtility::toJsNumber(family));

    return scope.Escape(obj);
}

NAN_MODULE_INIT(DebugProbe::Init)
{
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("DebugProbe").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    init(tpl);

    constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("DebugProbe").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(DebugProbe::New)
{
    if (info.IsConstructCall())
    {
        auto obj = new DebugProbe();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New(constructor);
        info.GetReturnValue().Set(cons->NewInstance());
    }
}

DebugProbe::DebugProbe()
{
    finderror = errorcodes::JsSuccess;

    NrfjprogErrorCodesType dll_find_result = OSFilesFindDll(dll_path, COMMON_MAX_PATH);
    NrfjprogErrorCodesType jlink_dll_find_result = OSFilesFindJLink(jlink_path, COMMON_MAX_PATH);

    if (dll_find_result != Success)
    {
        finderror = errorcodes::CouldNotFindJprogDLL;
    }
    else if (jlink_dll_find_result != Success)
    {
        finderror = errorcodes::CouldNotFindJlinkDLL;
    }
}

void DebugProbe::logCallback(const char * msg)
{
    logMessage = logMessage.append(msg);
}
NrfjprogErrorCodesType dll_load_result;

errorcodes DebugProbe::loadDll()
{
    if (loaded)
    {
        return errorcodes::JsSuccess;
    }

    if (finderror != errorcodes::JsSuccess)
    {
        return finderror;
    }

    dll_load_result = DllLoad(dll_path, &dll_function);
    loaded = false;

    if (dll_load_result != Success)
    {
        return errorcodes::CouldNotLoadDLL;
    }

    loaded = true;
    return errorcodes::JsSuccess;
}

DebugProbe::~DebugProbe()
{}

void DebugProbe::unloadDll()
{
    if (loaded)
    {
        loaded = false;
        DllFree();
    }
}

void DebugProbe::closeBeforeExit()
{
    dll_function.dll_close();

    unloadDll();
}

void DebugProbe::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "getDllVersion", GetDllVersion);
    Nan::SetPrototypeMethod(tpl, "getConnectedDevices", GetConnectedDevices);
    Nan::SetPrototypeMethod(tpl, "getFamily", GetFamily);
}

#pragma region GetDllVersion
NAN_METHOD(DebugProbe::GetDllVersion)
{
    auto obj = Nan::ObjectWrap::Unwrap<DebugProbe>(info.Holder());
    auto argumentCount = 0;
    v8::Local<v8::Function> callback;

    try
    {
        callback = ConversionUtility::getCallbackFunction(info[argumentCount]);
        argumentCount++;
    }
    catch (std::string error)
    {
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);
        return;
    }

    auto baton = new GetDllVersionBaton(callback);

    uv_queue_work(uv_default_loop(), baton->req, GetDllVersion, reinterpret_cast<uv_after_work_cb>(AfterGetDllVersion));
}


void DebugProbe::GetDllVersion(uv_work_t *req)
{
    auto baton = static_cast<GetDllVersionBaton*>(req->data);
    baton->result = errorcodes::JsSuccess;

    OPEN_BEFORE_CALL();
    nrfjprogdll_err_t error = dll_function.dll_get_version(&baton->major, &baton->minor, &baton->revision);
    EXIT_AFTER_CALL();

    if (error != Success)
    {
        baton->result = errorcodes::CouldNotCallFunction;
    }
}

void DebugProbe::AfterGetDllVersion(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<GetDllVersionBaton*>(req->data);
    v8::Local<v8::Value> argv[2];

    if (baton->result != errorcodes::JsSuccess)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "getting dll version");
        argv[1] = Nan::Undefined();
    }
    else
    {
        v8::Local<v8::Object> versionObj = Nan::New<v8::Object>();
        Utility::Set(versionObj, "major", ConversionUtility::toJsNumber(baton->major));
        Utility::Set(versionObj, "minor", ConversionUtility::toJsNumber(baton->minor));
        Utility::Set(versionObj, "revision", ConversionUtility::toJsNumber(baton->revision));

        argv[0] = Nan::Undefined();
        argv[1] = versionObj;
    }

    baton->callback->Call(2, argv);

    delete baton;
}
#pragma endregion GetFamily

#pragma region GetConnectedDevices
NAN_METHOD(DebugProbe::GetConnectedDevices)
{
    auto obj = Nan::ObjectWrap::Unwrap<DebugProbe>(info.Holder());
    auto argumentCount = 0;
    v8::Local<v8::Function> callback;

    try
    {
        callback = ConversionUtility::getCallbackFunction(info[argumentCount]);
        argumentCount++;
    }
    catch (std::string error)
    {
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);
        return;
    }

    auto baton = new GetConnectedDevicesBaton(callback);

    uv_queue_work(uv_default_loop(), baton->req, GetConnectedDevices, reinterpret_cast<uv_after_work_cb>(AfterGetConnectedDevices));
}


void DebugProbe::GetConnectedDevices(uv_work_t *req)
{
    auto baton = static_cast<GetConnectedDevicesBaton*>(req->data);
    baton->result = errorcodes::JsSuccess;

    OPEN_BEFORE_CALL();
    uint32_t serialNumbers[MAX_SERIAL_NUMBERS];
    uint32_t available = 0;
    nrfjprogdll_err_t error = dll_function.get_connected_probes(serialNumbers, MAX_SERIAL_NUMBERS, &available, 0);
    EXIT_AFTER_CALL();

    if (error != Success)
    {
        baton->result = errorcodes::CouldNotCallFunction;
        return;
    }

    for (uint32_t i = 0; i < available; i++)
    {
        baton->probes.push_back(new ProbeInfo(serialNumbers[i], NRF51_FAMILY));
    }
}

void DebugProbe::AfterGetConnectedDevices(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<GetConnectedDevicesBaton*>(req->data);
    v8::Local<v8::Value> argv[2];

    if (baton->result != errorcodes::JsSuccess)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "getting connected devices");
        argv[1] = Nan::Undefined();
    }
    else
    {
        v8::Local<v8::Array> connectedDevices = Nan::New<v8::Array>();
        for (uint32_t i = 0; i < baton->probes.size(); ++i)
        {
            Nan::Set(connectedDevices, ConversionUtility::toJsNumber(i), baton->probes[i]->ToJs());
        }

        argv[0] = Nan::Undefined();
        argv[1] = connectedDevices;
    }

    baton->callback->Call(2, argv);

    delete baton;
}
#pragma endregion GetConnectedDevices

#pragma region GetFamily
NAN_METHOD(DebugProbe::GetFamily)
{
    auto obj = Nan::ObjectWrap::Unwrap<DebugProbe>(info.Holder());
    auto argumentCount = 0;
    uint32_t serialNumber;
    v8::Local<v8::Function> callback;

    try
    {
        serialNumber = ConversionUtility::getNativeUint32(info[argumentCount]);
        argumentCount++;

        callback = ConversionUtility::getCallbackFunction(info[argumentCount]);
        argumentCount++;
    }
    catch (std::string error)
    {
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);
        return;
    }

    auto baton = new GetFamilyBaton(callback);
    baton->serialNumber = serialNumber;

    uv_queue_work(uv_default_loop(), baton->req, GetFamily, reinterpret_cast<uv_after_work_cb>(AfterGetFamily));
}


void DebugProbe::GetFamily(uv_work_t *req)
{
    auto baton = static_cast<GetFamilyBaton*>(req->data);
    baton->result = errorcodes::JsSuccess;
    Probe_handle_t probe;

    OPEN_BEFORE_CALL();
    INIT_PROBE(probe, baton->serialNumber);
    nrfjprogdll_err_t error = dll_function.get_device_family(probe, &baton->family);
    UNINIT_PROBE(probe);
    EXIT_AFTER_CALL();

    if (error != Success)
    {
        baton->result = errorcodes::CouldNotCallFunction;
        return;
    }
}

void DebugProbe::AfterGetFamily(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<GetFamilyBaton*>(req->data);
    v8::Local<v8::Value> argv[2];

    if (baton->result != errorcodes::JsSuccess)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "getting family");
        argv[1] = Nan::Undefined();
    }
    else
    {
        argv[0] = Nan::Undefined();
        argv[1] = ConversionUtility::toJsNumber(baton->family);
    }

    baton->callback->Call(2, argv);

    delete baton;
}
#pragma endregion GetFamily

extern "C" {
    void initConsts(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
    {/*
     NODE_DEFINE_CONSTANT(target, R0);
     NODE_DEFINE_CONSTANT(target, R1);
     NODE_DEFINE_CONSTANT(target, R2);
     NODE_DEFINE_CONSTANT(target, R3);
     NODE_DEFINE_CONSTANT(target, R4);
     NODE_DEFINE_CONSTANT(target, R5);
     NODE_DEFINE_CONSTANT(target, R6);
     NODE_DEFINE_CONSTANT(target, R7);
     NODE_DEFINE_CONSTANT(target, R8);
     NODE_DEFINE_CONSTANT(target, R9);
     NODE_DEFINE_CONSTANT(target, R10);
     NODE_DEFINE_CONSTANT(target, R11);
     NODE_DEFINE_CONSTANT(target, R12);
     NODE_DEFINE_CONSTANT(target, R13);
     NODE_DEFINE_CONSTANT(target, R14);
     NODE_DEFINE_CONSTANT(target, R15);
     NODE_DEFINE_CONSTANT(target, XPSR);
     NODE_DEFINE_CONSTANT(target, MSP);
     NODE_DEFINE_CONSTANT(target, PSP);

     NODE_DEFINE_CONSTANT(target, RAM_OFF);
     NODE_DEFINE_CONSTANT(target, RAM_ON);

     NODE_DEFINE_CONSTANT(target, NONE);
     NODE_DEFINE_CONSTANT(target, REGION_0);
     NODE_DEFINE_CONSTANT(target, ALL);
     NODE_DEFINE_CONSTANT(target, BOTH);

     NODE_DEFINE_CONSTANT(target, NO_REGION_0);
     NODE_DEFINE_CONSTANT(target, FACTORY);
     NODE_DEFINE_CONSTANT(target, USER);

     NODE_DEFINE_CONSTANT(target, UNKNOWN);
     NODE_DEFINE_CONSTANT(target, NRF51_XLR1);
     NODE_DEFINE_CONSTANT(target, NRF51_XLR2);
     NODE_DEFINE_CONSTANT(target, NRF51_XLR3);
     NODE_DEFINE_CONSTANT(target, NRF51_L3);
     NODE_DEFINE_CONSTANT(target, NRF51_XLR3P);
     NODE_DEFINE_CONSTANT(target, NRF51_XLR3LC);
     NODE_DEFINE_CONSTANT(target, NRF52_FP1_ENGA);
     NODE_DEFINE_CONSTANT(target, NRF52_FP1_ENGB);
     NODE_DEFINE_CONSTANT(target, NRF52_FP1);
     */
        NODE_DEFINE_CONSTANT(target, NRF51_FAMILY);
        NODE_DEFINE_CONSTANT(target, NRF52_FAMILY);
        /*
        NODE_DEFINE_CONSTANT(target, UP_DIRECTION);
        NODE_DEFINE_CONSTANT(target, DOWN_DIRECTION);

        NODE_DEFINE_CONSTANT(target, SUCCESS);
        NODE_DEFINE_CONSTANT(target, OUT_OF_MEMORY);
        NODE_DEFINE_CONSTANT(target, INVALID_OPERATION);
        NODE_DEFINE_CONSTANT(target, INVALID_PARAMETER);
        NODE_DEFINE_CONSTANT(target, INVALID_DEVICE_FOR_OPERATION);
        NODE_DEFINE_CONSTANT(target, WRONG_FAMILY_FOR_DEVICE);
        NODE_DEFINE_CONSTANT(target, EMULATOR_NOT_CONNECTED);
        NODE_DEFINE_CONSTANT(target, CANNOT_CONNECT);
        NODE_DEFINE_CONSTANT(target, LOW_VOLTAGE);
        NODE_DEFINE_CONSTANT(target, NO_EMULATOR_CONNECTED);
        NODE_DEFINE_CONSTANT(target, NVMC_ERROR);
        NODE_DEFINE_CONSTANT(target, NOT_AVAILABLE_BECAUSE_PROTECTION);
        NODE_DEFINE_CONSTANT(target, JLINKARM_DLL_NOT_FOUND);
        NODE_DEFINE_CONSTANT(target, JLINKARM_DLL_COULD_NOT_BE_OPENED);
        NODE_DEFINE_CONSTANT(target, JLINKARM_DLL_ERROR);
        NODE_DEFINE_CONSTANT(target, JLINKARM_DLL_TOO_OLD);
        NODE_DEFINE_CONSTANT(target, NRFJPROG_SUB_DLL_NOT_FOUND);
        NODE_DEFINE_CONSTANT(target, NRFJPROG_SUB_DLL_COULD_NOT_BE_OPENED);
        NODE_DEFINE_CONSTANT(target, NOT_IMPLEMENTED_ERROR);

        NODE_DEFINE_CONSTANT(target, Success);
        NODE_DEFINE_CONSTANT(target, NrfjprogError);
        NODE_DEFINE_CONSTANT(target, NrfjprogOutdatedError);
        NODE_DEFINE_CONSTANT(target, MemoryAllocationError);
        NODE_DEFINE_CONSTANT(target, InvalidArgumentError);
        NODE_DEFINE_CONSTANT(target, InsufficientArgumentsError);
        NODE_DEFINE_CONSTANT(target, IncompatibleArgumentsError);
        NODE_DEFINE_CONSTANT(target, DuplicatedArgumentsError);
        NODE_DEFINE_CONSTANT(target, NoOperationError);
        NODE_DEFINE_CONSTANT(target, UnavailableOperationBecauseProtectionError);
        NODE_DEFINE_CONSTANT(target, UnavailableOperationInFamilyError);
        NODE_DEFINE_CONSTANT(target, WrongFamilyForDeviceError);
        NODE_DEFINE_CONSTANT(target, NrfjprogDllNotFoundError);
        NODE_DEFINE_CONSTANT(target, NrfjprogDllLoadFailedError);
        NODE_DEFINE_CONSTANT(target, NrfjprogDllFunctionLoadFailedError);
        NODE_DEFINE_CONSTANT(target, NrfjprogDllNotImplementedError);
        NODE_DEFINE_CONSTANT(target, NrfjprogIniNotFoundError);
        NODE_DEFINE_CONSTANT(target, NrfjprogIniFormatError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllNotFoundError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllInvalidError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllFailedToOpenError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllTooOldError);
        NODE_DEFINE_CONSTANT(target, InvalidSerialNumberError);
        NODE_DEFINE_CONSTANT(target, NoDebuggersError);
        NODE_DEFINE_CONSTANT(target, NotPossibleToConnectError);
        NODE_DEFINE_CONSTANT(target, LowVoltageError);
        NODE_DEFINE_CONSTANT(target, FileNotFoundError);
        NODE_DEFINE_CONSTANT(target, InvalidHexFileError);
        NODE_DEFINE_CONSTANT(target, FicrReadError);
        NODE_DEFINE_CONSTANT(target, WrongArgumentError);
        NODE_DEFINE_CONSTANT(target, VerifyError);
        NODE_DEFINE_CONSTANT(target, NoWritePermissionError);
        NODE_DEFINE_CONSTANT(target, NVMCOperationError);
        NODE_DEFINE_CONSTANT(target, FlashNotErasedError);
        NODE_DEFINE_CONSTANT(target, RamIsOffError);
        NODE_DEFINE_CONSTANT(target, FicrOperationWarning);
        NODE_DEFINE_CONSTANT(target, UnalignedPageEraseWarning);
        NODE_DEFINE_CONSTANT(target, NoLogWarning);
        NODE_DEFINE_CONSTANT(target, UicrWriteOperationWithoutEraseWarning);
        */

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
        DebugProbe::Init(target);
    }
}

NODE_MODULE(pc_nrfjprog, init);
