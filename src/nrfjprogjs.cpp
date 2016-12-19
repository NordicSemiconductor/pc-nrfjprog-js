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

#include "keilhexfile.h"
#include "dllfunc.h"

#include <iostream>

Nan::Persistent<v8::Function> DebugProbe::constructor;
DllFunctionPointersType DebugProbe::dll_function;
char DebugProbe::dll_path[COMMON_MAX_PATH] = {'\0'};
char DebugProbe::jlink_path[COMMON_MAX_PATH] = {'\0'};
bool DebugProbe::loaded = false;
bool DebugProbe::connectedToDevice = false;
int DebugProbe::error = errorcodes::JsSuccess;
int DebugProbe::finderror = errorcodes::JsSuccess;
uint32_t DebugProbe::emulatorSpeed = 1000;
uint32_t DebugProbe::versionMagicNumber = 0x46D8A517; //YGGDRAIL

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

void DebugProbe::loadDll()
{
    if (loaded)
    {
        return;
    }

    if (finderror != errorcodes::JsSuccess)
    {
        error = finderror;
        return;
    }

    NrfjprogErrorCodesType dll_load_result = DllLoad(dll_path, &dll_function);
    loaded = false;

    if (dll_load_result != Success)
    {
        error = errorcodes::CouldNotLoadDLL;
    }
    else
    {
        error = errorcodes::JsSuccess;
        loaded = true;
    }
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
    if (connectedToDevice)
    {
        dll_function.sys_reset();
        dll_function.go();

        dll_function.disconnect_from_emu();

        connectedToDevice = false;
    }

    dll_function.close_dll();

    unloadDll();
}

void DebugProbe::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "program", Program);
    Nan::SetPrototypeMethod(tpl, "getVersion", GetVersion);
    Nan::SetPrototypeMethod(tpl, "getSerialNumbers", GetSerialnumbers);
    Nan::SetPrototypeMethod(tpl, "readAddress", ReadAddress);
}

device_family_t DebugProbe::openDll(device_family_t family, const uint32_t serialnumber)
{
    nrfjprogdll_err_t err = SUCCESS;

    if (family == ANY_FAMILY || family == NRF51_FAMILY)
    {
        err = dll_function.open_dll(jlink_path, nullptr, NRF51_FAMILY);

        if (err != SUCCESS)
        {
            error = errorcodes::CouldNotOpenDevice;
            return ANY_FAMILY;
        }

        if (correctFamily(serialnumber) == SUCCESS)
        {
            return NRF51_FAMILY;
        }

        dll_function.close_dll();
    }

    if (family == ANY_FAMILY || family == NRF52_FAMILY)
    {
        if (dll_function.open_dll(jlink_path, nullptr, NRF52_FAMILY) != SUCCESS)
        {
            error = errorcodes::CouldNotOpenDevice;
            return ANY_FAMILY;
        }

        if (correctFamily(serialnumber) == SUCCESS)
        {
            return NRF52_FAMILY;
        }

        dll_function.close_dll();
    }

    return ANY_FAMILY;
}

nrfjprogdll_err_t DebugProbe::correctFamily(const uint32_t serialnumber)
{
    if (serialnumber == 0)
    {
        return SUCCESS;
    }

    device_version_t deviceType = UNKNOWN;

    nrfjprogdll_err_t err = dll_function.connect_to_emu_with_snr(serialnumber, emulatorSpeed);

    if (err != SUCCESS)
    {
        return err;
    }

    connectedToDevice = true;

    err = dll_function.read_device_version(&deviceType);
    dll_function.disconnect_from_emu();

    connectedToDevice = false;

    if (err != SUCCESS)
    {
        return err;
    }

    return err;
}

#pragma region GetSerialnumbers
NAN_METHOD(DebugProbe::GetSerialnumbers)
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

    auto baton = new GetSerialnumbersBaton(callback);

    uv_queue_work(uv_default_loop(), baton->req, GetSerialnumbers, reinterpret_cast<uv_after_work_cb>(AfterGetSerialnumbers));
}


void DebugProbe::GetSerialnumbers(uv_work_t *req)
{
    auto baton = static_cast<GetSerialnumbersBaton*>(req->data);

    loadDll();

    if (!loaded)
    {
        baton->result = error;
        unloadDll();
        return;
    }

    auto const probe_count_max = MAX_SERIAL_NUMBERS;
    uint32_t probe_count = 0;

    uint32_t _probes[MAX_SERIAL_NUMBERS];

    if (openDll(NRF51_FAMILY, 0) == ANY_FAMILY)
    {
        baton->result = error;
        unloadDll();
        return;
    }

    baton->result = dll_function.enum_emu_snr(_probes, probe_count_max, &probe_count);

    if (baton->result != SUCCESS)
    {
        closeBeforeExit();
        baton->result = errorcodes::CouldNotCallFunction;
        return;
    }

    for (uint32_t i = 0; i < probe_count; i++)
    {
        device_family_t family = NRF51_FAMILY;

        if (correctFamily(_probes[i]) == WRONG_FAMILY_FOR_DEVICE)
        {
            family = NRF52_FAMILY;
        }

        baton->probes.push_back(new ProbeInfo(_probes[i], family));
    }

    closeBeforeExit();
}

void DebugProbe::AfterGetSerialnumbers(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<GetSerialnumbersBaton*>(req->data);
    v8::Local<v8::Value> argv[2];

    if (baton->result != errorcodes::JsSuccess)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "getting serialnumbers");
        argv[1] = Nan::Undefined();
    }
    else
    {
        argv[0] = Nan::Undefined();
        v8::Local<v8::Array> serialNumbers = Nan::New<v8::Array>();

        for (uint32_t i = 0; i < baton->probes.size(); ++i)
        {
            Nan::Set(serialNumbers, Nan::New<v8::Integer>(i), baton->probes[i]->ToJs());
        }

        argv[1] = serialNumbers;
    }

    baton->callback->Call(2, argv);

    delete baton;
}
#pragma endregion GetSerialnumbers

#pragma region Program
NAN_METHOD(DebugProbe::Program)
{
    auto obj = Nan::ObjectWrap::Unwrap<DebugProbe>(info.Holder());
    auto argumentCount = 0;

    uint32_t serialNumber;
    uint32_t family = ANY_FAMILY;
    std::string filename;
    v8::Local<v8::Object> filenameObject;
    v8::Local<v8::Function> callback;

    try
    {
        serialNumber = ConversionUtility::getNativeUint32(info[argumentCount]);
        argumentCount++;

        if (info.Length() == 3)
        {
            filenameObject = ConversionUtility::getJsObject(info[argumentCount]);
            argumentCount++;
        }
        else if (info.Length() == 4)
        {
            family = ConversionUtility::getNativeUint32(info[argumentCount]);
            argumentCount++;

            filename = ConversionUtility::getNativeString(info[argumentCount]);
            argumentCount++;
        }
        else
        {
            throw "parameter count";
        }

        callback = ConversionUtility::getCallbackFunction(info[argumentCount]);
        argumentCount++;
    }
    catch (std::string error)
    {
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);
        return;
    }

    auto baton = new ProgramBaton(callback);
    baton->serialnumber = serialNumber;
    baton->family = (device_family_t)family;

    if (info.Length() == 4)
    {
        baton->filenameMap[baton->family] = filename;
    }
    else
    {
        if (Utility::Has(filenameObject, "filecontent"))
        {
            baton->filecontent = ConversionUtility::getNativeBool(Utility::Get(filenameObject, "filecontent"));
        }
        else
        {
            baton->filecontent = false;
        }

        if (Utility::Has(filenameObject, NRF51_FAMILY))
        {
            baton->filenameMap[NRF51_FAMILY] = ConversionUtility::getNativeString(Utility::Get(filenameObject, NRF51_FAMILY));
        }

        if (Utility::Has(filenameObject, NRF52_FAMILY))
        {
            baton->filenameMap[NRF52_FAMILY] = ConversionUtility::getNativeString(Utility::Get(filenameObject, NRF52_FAMILY));
        }
    }

    uv_queue_work(uv_default_loop(), baton->req, Program, reinterpret_cast<uv_after_work_cb>(AfterProgram));
}

void DebugProbe::Program(uv_work_t *req)
{
    auto baton = static_cast<ProgramBaton*>(req->data);

    loadDll();

    if (!loaded)
    {
        baton->result = error;
        return;
    }

    baton->family = openDll(baton->family, baton->serialnumber);

    if (baton->family == ANY_FAMILY)
    {
        baton->result = error;
        unloadDll();
        return;
    }

    baton->filename = baton->filenameMap[baton->family];

    baton->result = dll_function.connect_to_emu_with_snr(baton->serialnumber, emulatorSpeed);

    if (baton->result != SUCCESS)
    {
        baton->result = errorcodes::CouldNotConnectToDevice;
        closeBeforeExit();
        return;
    }

    connectedToDevice = true;

    KeilHexFile program_hex;
    KeilHexFile::Status status;

    if (baton->filecontent)
    {
        status = program_hex.open(baton->filename);
    }
    else
    {
        status = program_hex.open(baton->filename.c_str());
    }

    if (status != SUCCESS)
    {
        baton->result = errorcodes::CouldNotOpenHexFile;
        return;
    }

    uint32_t code_size = 512 * 1024;
    uint8_t *code = new uint8_t[code_size];

    if (program_hex.nand_read(0, code, code_size) != KeilHexFile::SUCCESS)
    {
        baton->result = errorcodes::CouldNotCallFunction;
        closeBeforeExit();
        delete[] code;
        return;
    }

    if (dll_function.erase_all() != SUCCESS)
    {
        baton->result = errorcodes::CouldNotErase;
        closeBeforeExit();
        delete[] code;
        return;
    }

    uint32_t foundAddress;
    uint32_t bytesFound;

    program_hex.find_contiguous(0, foundAddress, bytesFound);

    do
    {
        baton->result = dll_function.write(foundAddress, (const uint8_t *)&code[foundAddress], bytesFound, true);

        if (baton->result != SUCCESS)
        {
            baton->result = errorcodes::CouldNotProgram;
            break;
        }

        program_hex.find_contiguous(foundAddress + bytesFound, foundAddress, bytesFound);
    } while (bytesFound != 0);

    closeBeforeExit();
    delete[] code;
}

void DebugProbe::AfterProgram(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<ProgramBaton*>(req->data);
    v8::Local<v8::Value> argv[1];

    if (baton->result != errorcodes::JsSuccess)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "programming");
    }
    else
    {
        argv[0] = Nan::Undefined();
    }

    baton->callback->Call(1, argv);

    delete baton;
}
#pragma endregion Program

#pragma region GetVersion
NAN_METHOD(DebugProbe::GetVersion)
{
    auto obj = Nan::ObjectWrap::Unwrap<DebugProbe>(info.Holder());
    auto argumentCount = 0;

    uint32_t serialNumber;
    uint32_t family = ANY_FAMILY;
    v8::Local<v8::Function> callback;

    try
    {
        serialNumber = ConversionUtility::getNativeUint32(info[argumentCount]);
        argumentCount++;

        if (info.Length() == 3)
        {
            family = ConversionUtility::getNativeUint32(info[argumentCount]);
            argumentCount++;
        }

        callback = ConversionUtility::getCallbackFunction(info[argumentCount]);
        argumentCount++;
    }
    catch (std::string error)
    {
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);
        return;
    }

    auto baton = new GetVersionBaton(callback);
    baton->serialnumber = serialNumber;
    baton->family = (device_family_t)family;

    uv_queue_work(uv_default_loop(), baton->req, GetVersion, reinterpret_cast<uv_after_work_cb>(AfterGetVersion));
}

uint32_t getNumber(const uint8_t *data, const int offset, const int length)
{
    uint32_t _r = 0;

    for (int i = 0; i < length; i++)
    {
        _r += (data[offset + i] << (i * 8));
    }

    return _r;
}

void DebugProbe::GetVersion(uv_work_t *req)
{
    auto baton = static_cast<GetVersionBaton*>(req->data);

    loadDll();

    if (!loaded)
    {
        baton->result = error;
        return;
    }

    baton->family = openDll(baton->family, baton->serialnumber);

    if (baton->family == ANY_FAMILY)
    {
        baton->result = error;
        unloadDll();
        return;
    }

    baton->result = dll_function.connect_to_emu_with_snr(baton->serialnumber, emulatorSpeed);

    if (baton->result != SUCCESS)
    {
        baton->result = errorcodes::CouldNotConnectToDevice;
        closeBeforeExit();
        return;
    }

    connectedToDevice = true;

    baton->result = dll_function.read(0x20000, baton->versionData, 24);

    if (baton->result != SUCCESS)
    {
        baton->result = errorcodes::CouldNotRead;
        closeBeforeExit();
        return;
    }

    const uint32_t magicNumber = getNumber(baton->versionData, 0, 4);

    if (magicNumber != versionMagicNumber)
    {
        std::cout << std::hex << magicNumber << std::endl;
        baton->result = errorcodes::WrongMagicNumber;
        closeBeforeExit();
        return;
    }

    closeBeforeExit();
}

#include <sstream>

void DebugProbe::AfterGetVersion(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<GetVersionBaton*>(req->data);
    v8::Local<v8::Value> argv[2];

    if (baton->result != errorcodes::JsSuccess)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "getting version");
        argv[1] = Nan::Undefined();
    }
    else
    {
        argv[0] = Nan::Undefined();

        v8::Local<v8::Object> obj = Nan::New<v8::Object>();

        uint32_t magic = getNumber(baton->versionData, 0, 4);
        uint8_t structVersion = getNumber(baton->versionData, 4, 1);
        uint32_t hash = getNumber(baton->versionData, 8, 4);
        uint8_t major = getNumber(baton->versionData, 12, 1);
        uint8_t minor = getNumber(baton->versionData, 13, 1);
        uint8_t patch = getNumber(baton->versionData, 14, 1);
        uint8_t sdBleApiVersion = getNumber(baton->versionData, 16, 1);
        uint8_t transportType = getNumber(baton->versionData, 17, 1);
        uint32_t baudRate = getNumber(baton->versionData, 20, 4);

        std::stringstream versionstring;
        std::stringstream versionstringComplete;

        versionstring << (int)major << "." << (int)minor << "." << (int)patch;
        versionstringComplete << (int)major << "." << (int)minor << "." << (int)patch << " " << std::hex << hash;

        std::stringstream magicString;

        magicString << std::hex << magic;

        Utility::Set(obj, "magic", ConversionUtility::toJsString(magicString.str()));
        Utility::Set(obj, "firmwareID", Nan::Undefined()); // Keep for backward compatibility
        Utility::Set(obj, "structVersion", ConversionUtility::toJsNumber(structVersion));
        Utility::Set(obj, "major", ConversionUtility::toJsNumber(major));
        Utility::Set(obj, "minor", ConversionUtility::toJsNumber(minor));
        Utility::Set(obj, "patch", ConversionUtility::toJsNumber(patch));
        Utility::Set(obj, "hash", ConversionUtility::toJsNumber(hash));
        Utility::Set(obj, "string", ConversionUtility::toJsString(versionstring.str()));
        Utility::Set(obj, "stringComplete", ConversionUtility::toJsString(versionstringComplete.str()));

        if (structVersion == 2)
        {
            Utility::Set(obj, "sdBleApiVersion", ConversionUtility::toJsNumber(sdBleApiVersion));
            Utility::Set(obj, "transportType", ConversionUtility::toJsNumber(transportType));
            Utility::Set(obj, "baudRate", ConversionUtility::toJsNumber(baudRate));
        }
        else
        {
            Utility::Set(obj, "sdBleApiVersion", Nan::Undefined());
            Utility::Set(obj, "transportType", Nan::Undefined());
            Utility::Set(obj, "baudRate", Nan::Undefined());
        }

        argv[1] = obj;
    }

    baton->callback->Call(2, argv);

    delete baton;
}
#pragma endregion GetVersion

#pragma region ReadAddress
NAN_METHOD(DebugProbe::ReadAddress)
{
    auto obj = Nan::ObjectWrap::Unwrap<DebugProbe>(info.Holder());
    auto argumentCount = 0;

    uint32_t serialNumber;
    uint32_t family = ANY_FAMILY;
    uint32_t address = 0;
    uint32_t length = 0;
    v8::Local<v8::Function> callback;

    try
    {
        serialNumber = ConversionUtility::getNativeUint32(info[argumentCount]);
        argumentCount++;

        if (info.Length() == 5)
        {
            family = ConversionUtility::getNativeUint32(info[argumentCount]);
            argumentCount++;
        }

        address = ConversionUtility::getNativeUint32(info[argumentCount]);
        argumentCount++;

        length = ConversionUtility::getNativeUint32(info[argumentCount]);
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

    auto baton = new ReadAddressBaton(callback);
    baton->serialnumber = serialNumber;
    baton->family = (device_family_t)family;
    baton->address = address;
    baton->length = length;
    baton->data = new uint8_t[length];

    uv_queue_work(uv_default_loop(), baton->req, ReadAddress, reinterpret_cast<uv_after_work_cb>(AfterReadAddress));
}

void DebugProbe::ReadAddress(uv_work_t *req)
{
    auto baton = static_cast<ReadAddressBaton*>(req->data);

    loadDll();

    if (!loaded)
    {
        baton->result = error;
        return;
    }

    baton->family = openDll(baton->family, baton->serialnumber);

    if (baton->family == ANY_FAMILY)
    {
        baton->result = error;
        unloadDll();
        return;
    }

    baton->result = dll_function.connect_to_emu_with_snr(baton->serialnumber, emulatorSpeed);

    if (baton->result != SUCCESS)
    {
        baton->result = errorcodes::CouldNotConnectToDevice;
        closeBeforeExit();
        return;
    }

    connectedToDevice = true;

    baton->result = dll_function.read(baton->address, baton->data, baton->length);

    if (baton->result != SUCCESS)
    {
        baton->result = errorcodes::CouldNotRead;
        closeBeforeExit();
        return;
    }

    closeBeforeExit();
}

void DebugProbe::AfterReadAddress(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<ReadAddressBaton*>(req->data);
    v8::Local<v8::Value> argv[2];

    if (baton->result != errorcodes::JsSuccess)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "reading address");
        argv[1] = Nan::Undefined();
    }
    else
    {
        argv[0] = Nan::Undefined();

        argv[1] = ConversionUtility::toJsValueArray(baton->data, baton->length);
    }

    baton->callback->Call(2, argv);

    delete[] baton->data;
    delete baton;
}
#pragma endregion ReadAddress

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
        NODE_DEFINE_CONSTANT(target, CouldNotOpenDevice);
        NODE_DEFINE_CONSTANT(target, CouldNotLoadDLL);
        NODE_DEFINE_CONSTANT(target, CouldNotCallFunction);
    }

    NAN_MODULE_INIT(init)
    {
        initConsts(target);
        DebugProbe::Init(target);
    }
}

NODE_MODULE(pc_nrfjprog, init);
