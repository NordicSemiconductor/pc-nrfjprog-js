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

#include <chrono>
#include <sstream>
#include <iostream>

#include "DllCommonDefinitions.h"
#include "nrfjprog.h"

#include "common.h"

#define RETURN_VALUE_OR_THROW_EXCEPTION(method) \
try { \
    return (method); \
} \
catch(char const *error) \
{ \
    std::cout << "Exception: " << name << ":" << error << std::endl; \
    std::stringstream ex; \
    ex << "Failed to get property " << name << ": " << error; \
    throw ex.str(); \
}

static name_map_t cpu_registers_map = {
    NAME_MAP_ENTRY(R0),
	NAME_MAP_ENTRY(R1),
	NAME_MAP_ENTRY(R2),
	NAME_MAP_ENTRY(R3),
	NAME_MAP_ENTRY(R4),
	NAME_MAP_ENTRY(R5),
	NAME_MAP_ENTRY(R6),
	NAME_MAP_ENTRY(R7),
	NAME_MAP_ENTRY(R8),
	NAME_MAP_ENTRY(R9),
	NAME_MAP_ENTRY(R10),
	NAME_MAP_ENTRY(R11),
	NAME_MAP_ENTRY(R12),
	NAME_MAP_ENTRY(R13),
	NAME_MAP_ENTRY(R14),
	NAME_MAP_ENTRY(R15),
	NAME_MAP_ENTRY(XPSR),
	NAME_MAP_ENTRY(MSP),
	NAME_MAP_ENTRY(PSP)
};

static name_map_t ram_section_power_status_map = {
    NAME_MAP_ENTRY(RAM_OFF),
    NAME_MAP_ENTRY(RAM_ON)
};

static name_map_t readback_protection_status_map = {
    NAME_MAP_ENTRY(NONE),
	NAME_MAP_ENTRY(REGION_0),
	NAME_MAP_ENTRY(ALL),
	NAME_MAP_ENTRY(BOTH)
};

static name_map_t region_0_source_map = {
    NAME_MAP_ENTRY(NO_REGION_0),
    NAME_MAP_ENTRY(FACTORY),
    NAME_MAP_ENTRY(USER)
};

static name_map_t device_version_map = {
    NAME_MAP_ENTRY(UNKNOWN),
    NAME_MAP_ENTRY(NRF51_XLR1),
    NAME_MAP_ENTRY(NRF51_XLR2),
    NAME_MAP_ENTRY(NRF51_XLR3),
    NAME_MAP_ENTRY(NRF51_L3),
    NAME_MAP_ENTRY(NRF51_XLR3P),
    NAME_MAP_ENTRY(NRF51_XLR3LC),
    NAME_MAP_ENTRY(NRF52_FP1_ENGA),
    NAME_MAP_ENTRY(NRF52_FP1_ENGB),
    NAME_MAP_ENTRY(NRF52_FP1)
};

static name_map_t device_family_map = {
    NAME_MAP_ENTRY(NRF51_FAMILY),
    NAME_MAP_ENTRY(NRF52_FAMILY)
};

static name_map_t rtt_direction_map = {
    NAME_MAP_ENTRY(UP_DIRECTION),
    NAME_MAP_ENTRY(DOWN_DIRECTION)
};

static name_map_t nrfjprogdll_err_map = {
    NAME_MAP_ENTRY(SUCCESS),
    NAME_MAP_ENTRY(OUT_OF_MEMORY),
    NAME_MAP_ENTRY(INVALID_OPERATION),
    NAME_MAP_ENTRY(INVALID_PARAMETER),
    NAME_MAP_ENTRY(INVALID_DEVICE_FOR_OPERATION),
    NAME_MAP_ENTRY(WRONG_FAMILY_FOR_DEVICE),
    NAME_MAP_ENTRY(EMULATOR_NOT_CONNECTED),
    NAME_MAP_ENTRY(CANNOT_CONNECT),
    NAME_MAP_ENTRY(LOW_VOLTAGE),
    NAME_MAP_ENTRY(NO_EMULATOR_CONNECTED),
    NAME_MAP_ENTRY(NVMC_ERROR),
    NAME_MAP_ENTRY(NOT_AVAILABLE_BECAUSE_PROTECTION),
    NAME_MAP_ENTRY(JLINKARM_DLL_NOT_FOUND),
    NAME_MAP_ENTRY(JLINKARM_DLL_COULD_NOT_BE_OPENED),
    NAME_MAP_ENTRY(JLINKARM_DLL_ERROR),
    NAME_MAP_ENTRY(JLINKARM_DLL_TOO_OLD),
    NAME_MAP_ENTRY(NRFJPROG_SUB_DLL_NOT_FOUND),
    NAME_MAP_ENTRY(NRFJPROG_SUB_DLL_COULD_NOT_BE_OPENED),
    NAME_MAP_ENTRY(NOT_IMPLEMENTED_ERROR)
};

static name_map_t NrfjprogErrorCodesTypeMap = {
    NAME_MAP_ENTRY(Success),
    NAME_MAP_ENTRY(NrfjprogError),
    NAME_MAP_ENTRY(NrfjprogOutdatedError),
    NAME_MAP_ENTRY(MemoryAllocationError),
    NAME_MAP_ENTRY(InvalidArgumentError),
    NAME_MAP_ENTRY(InsufficientArgumentsError),
    NAME_MAP_ENTRY(IncompatibleArgumentsError),
    NAME_MAP_ENTRY(DuplicatedArgumentsError),
    NAME_MAP_ENTRY(NoOperationError),
    NAME_MAP_ENTRY(UnavailableOperationBecauseProtectionError),
    NAME_MAP_ENTRY(UnavailableOperationInFamilyError),
    NAME_MAP_ENTRY(WrongFamilyForDeviceError),
    NAME_MAP_ENTRY(NrfjprogDllNotFoundError),
    NAME_MAP_ENTRY(NrfjprogDllLoadFailedError),
    NAME_MAP_ENTRY(NrfjprogDllFunctionLoadFailedError),
    NAME_MAP_ENTRY(NrfjprogDllNotImplementedError),
    NAME_MAP_ENTRY(NrfjprogIniNotFoundError),
    NAME_MAP_ENTRY(NrfjprogIniFormatError),
    NAME_MAP_ENTRY(JLinkARMDllNotFoundError),
    NAME_MAP_ENTRY(JLinkARMDllInvalidError),
    NAME_MAP_ENTRY(JLinkARMDllFailedToOpenError),
    NAME_MAP_ENTRY(JLinkARMDllError),
    NAME_MAP_ENTRY(JLinkARMDllTooOldError),
    NAME_MAP_ENTRY(InvalidSerialNumberError),
    NAME_MAP_ENTRY(NoDebuggersError),
    NAME_MAP_ENTRY(NotPossibleToConnectError),
    NAME_MAP_ENTRY(LowVoltageError),
    NAME_MAP_ENTRY(FileNotFoundError),
    NAME_MAP_ENTRY(InvalidHexFileError),
    NAME_MAP_ENTRY(FicrReadError),
    NAME_MAP_ENTRY(WrongArgumentError),
    NAME_MAP_ENTRY(VerifyError),
    NAME_MAP_ENTRY(NoWritePermissionError),
    NAME_MAP_ENTRY(NVMCOperationError),
    NAME_MAP_ENTRY(FlashNotErasedError),
    NAME_MAP_ENTRY(RamIsOffError),
    NAME_MAP_ENTRY(FicrOperationWarning),
    NAME_MAP_ENTRY(UnalignedPageEraseWarning),
    NAME_MAP_ENTRY(NoLogWarning),
    NAME_MAP_ENTRY(UicrWriteOperationWithoutEraseWarning)
};

static name_map_t nrfjprog_js_err_map = {
    { errorcodes::JsSuccess, "Success" },
    { errorcodes::CouldNotFindJlinkDLL, "CouldNotFindJlinkDLL" },
    { errorcodes::CouldNotFindJprogDLL, "CouldNotFindJprogDLL" },
    { errorcodes::CouldNotLoadDLL, "CouldNotLoadDLL" },
    { errorcodes::CouldNotOpenDevice, "CouldNotOpenDevice" },
    { errorcodes::CouldNotConnectToDevice, "CouldNotConnectToDevice" },
    { errorcodes::CouldNotCallFunction, "CouldNotCallFunction" },
    { errorcodes::CouldNotErase, "CouldNotErase" },
    { errorcodes::CouldNotProgram, "CouldNotProgram" },
    { errorcodes::CouldNotRead, "CouldNotRead" },
    { errorcodes::CouldNotOpenHexFile, "CouldNotOpenHexFile" },
    { errorcodes::WrongMagicNumber, "WrongMagicNumber" }
};

const std::string getCurrentTimeInMilliseconds()
{
    auto current_time = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(current_time);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch());

    auto ttm = gmtime(&time);

    char date_time_format[] = "%Y-%m-%dT%H:%M:%S";
    char time_str[20] = "";

    strftime(time_str, 20, date_time_format, ttm);

    std::string result(time_str);
    result.append(".");

    char millisecond_str[4];
    sprintf(millisecond_str, "%03d", static_cast<int>(ms.count() % 1000));
    result.append(millisecond_str);
    result.append("Z");

    return result;
}

uint16_t uint16_decode(const uint8_t *p_encoded_data)
{
        return ( (static_cast<uint16_t>(const_cast<uint8_t *>(p_encoded_data)[0])) |
                 (static_cast<uint16_t>(const_cast<uint8_t *>(p_encoded_data)[1]) << 8 ));
}

uint32_t uint32_decode(const uint8_t *p_encoded_data)
{
    return ((static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[0]) << 0)  |
            (static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[1]) << 8)  |
            (static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[2]) << 16) |
            (static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[3]) << 24));
}

uint16_t fromNameToValue(name_map_t names, const char *name)
{
    name_map_it_t it;
    uint16_t key = -1;

    for (it = names.begin(); it != names.end(); ++it)
    {
        if (strcmp(it->second, name) == 0)
        {
            key = it->first;
            break;
        }
    }

    return key;
}


uint32_t ConversionUtility::getNativeUint32(v8::Local<v8::Object>js, const char *name)
{
    RETURN_VALUE_OR_THROW_EXCEPTION(ConvUtil<uint32_t>::getNativeUnsigned(js, name));
}

uint32_t ConversionUtility::getNativeUint32(v8::Local<v8::Value> js)
{
    return ConvUtil<uint32_t>::getNativeUnsigned(js);
}

uint16_t ConversionUtility::getNativeUint16(v8::Local<v8::Object>js, const char *name)
{
    RETURN_VALUE_OR_THROW_EXCEPTION(ConvUtil<uint16_t>::getNativeUnsigned(js, name));
}

uint16_t ConversionUtility::getNativeUint16(v8::Local<v8::Value> js)
{
    return ConvUtil<uint16_t>::getNativeUnsigned(js);
}

uint8_t ConversionUtility::getNativeUint8(v8::Local<v8::Object>js, const char *name)
{
    RETURN_VALUE_OR_THROW_EXCEPTION(ConvUtil<uint8_t>::getNativeUnsigned(js, name));
}

uint8_t ConversionUtility::getNativeUint8(v8::Local<v8::Value> js)
{
    return ConvUtil<uint8_t>::getNativeUnsigned(js);
}

int32_t ConversionUtility::getNativeInt32(v8::Local<v8::Object>js, const char *name)
{
    RETURN_VALUE_OR_THROW_EXCEPTION(ConvUtil<int32_t>::getNativeSigned(js, name));
}

int32_t ConversionUtility::getNativeInt32(v8::Local<v8::Value>js)
{
    return ConvUtil<int32_t>::getNativeSigned(js);
}

int16_t ConversionUtility::getNativeInt16(v8::Local<v8::Object>js, const char *name)
{
    RETURN_VALUE_OR_THROW_EXCEPTION(ConvUtil<int16_t>::getNativeSigned(js, name));
}

int16_t ConversionUtility::getNativeInt16(v8::Local<v8::Value>js)
{
    return ConvUtil<int16_t>::getNativeSigned(js);
}

int8_t ConversionUtility::getNativeInt8(v8::Local<v8::Object>js, const char *name)
{
    RETURN_VALUE_OR_THROW_EXCEPTION(ConvUtil<int8_t>::getNativeSigned(js, name));
}

int8_t ConversionUtility::getNativeInt8(v8::Local<v8::Value>js)
{
    return ConvUtil<int8_t>::getNativeSigned(js);
}

double ConversionUtility::getNativeDouble(v8::Local<v8::Object>js, const char *name)
{
    RETURN_VALUE_OR_THROW_EXCEPTION(ConvUtil<double>::getNativeFloat(js, name));
}

double ConversionUtility::getNativeDouble(v8::Local<v8::Value>js)
{
    return ConvUtil<double>::getNativeFloat(js);
}

uint8_t ConversionUtility::getNativeBool(v8::Local<v8::Object>js, const char *name)
{
    RETURN_VALUE_OR_THROW_EXCEPTION(ConvUtil<bool>::getNativeBool(js, name));
}

uint8_t ConversionUtility::getNativeBool(v8::Local<v8::Value>js)
{
    return ConvUtil<bool>::getNativeBool(js);
}

bool ConversionUtility::getBool(v8::Local<v8::Object>js, const char *name)
{
    RETURN_VALUE_OR_THROW_EXCEPTION(ConvUtil<bool>::getNativeBool(js, name));
}

bool ConversionUtility::getBool(v8::Local<v8::Value>js)
{
    return ConvUtil<bool>::getNativeBool(js);
}

uint8_t *ConversionUtility::getNativePointerToUint8(v8::Local<v8::Object>js, const char *name)
{
    v8::Local<v8::Value> value = Utility::Get(js, name);

    RETURN_VALUE_OR_THROW_EXCEPTION(ConversionUtility::getNativePointerToUint8(value));
}

uint8_t *ConversionUtility::getNativePointerToUint8(v8::Local<v8::Value> js)
{
    if (!js->IsArray())
    {
        throw "array";
    }

    v8::Local<v8::Array> jsarray = v8::Local<v8::Array>::Cast(js);
    auto length = jsarray->Length();
    auto string = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * length));

    assert(string != nullptr);

    for (uint32_t i = 0; i < length; ++i)
    {
        string[i] = static_cast<uint8_t>(jsarray->Get(Nan::New(i))->Uint32Value());
    }

    return string;
}

uint16_t *ConversionUtility::getNativePointerToUint16(v8::Local<v8::Object>js, const char *name)
{
    v8::Local<v8::Value> value = Utility::Get(js, name);

    RETURN_VALUE_OR_THROW_EXCEPTION(ConversionUtility::getNativePointerToUint16(value));
}

uint16_t *ConversionUtility::getNativePointerToUint16(v8::Local<v8::Value>js)
{
    v8::Local<v8::Array> jsarray = v8::Local<v8::Array>::Cast(js);
    auto length = jsarray->Length();
    auto string = static_cast<uint16_t *>(malloc(sizeof(uint16_t) * length));

    assert(string != nullptr);

    for (uint32_t i = 0; i < length; ++i)
    {
        string[i] = static_cast<uint16_t>(jsarray->Get(Nan::New(i))->Uint32Value());
    }

    return string;
}

v8::Local<v8::Object> ConversionUtility::getJsObject(v8::Local<v8::Value>js)
{
    if (!js->IsObject())
    {
        throw "object";
    }

    return js->ToObject();
}

v8::Local<v8::Object> ConversionUtility::getJsObject(v8::Local<v8::Object> js, const char *name)
{
    v8::Local<v8::Value> obj = Utility::Get(js, name);

    RETURN_VALUE_OR_THROW_EXCEPTION(ConversionUtility::getJsObject(obj));
}

v8::Local<v8::Object> ConversionUtility::getJsObjectOrNull(v8::Local<v8::Value>js)
{
    if (js->IsNull())
    {
        Nan::EscapableHandleScope scope;
        v8::Local<v8::Object> newobj = Nan::New<v8::Object>();
        Utility::Set(newobj, "special_hack_null_object", true);
        return scope.Escape(newobj);
    }
    else if (js->IsObject())
    {
        return ConversionUtility::getJsObject(js);
    }

    throw "object or null";
}

v8::Local<v8::Object> ConversionUtility::getJsObjectOrNull(v8::Local<v8::Object>js, const char *name)
{
    v8::Local<v8::Value> obj = Utility::Get(js, name);

    RETURN_VALUE_OR_THROW_EXCEPTION(ConversionUtility::getJsObjectOrNull(obj));
}

uint16_t ConversionUtility::stringToValue(name_map_t name_map, v8::Local<v8::Object> string, uint16_t defaultValue)
{
    name_map_it_t it;
    auto key = defaultValue;

    auto name = reinterpret_cast<const char *>(ConversionUtility::getNativePointerToUint8(string));

    for (it = name_map.begin(); it != name_map.end(); ++it)
    {
        if (strcmp(it->second, name) == 0)
        {
            key = it->first;
            break;
        }
    }

    return key;
}

std::string ConversionUtility::getNativeString(v8::Local<v8::Object>js, const char *name)
{
    v8::Local<v8::Value> obj = Utility::Get(js, name);

    RETURN_VALUE_OR_THROW_EXCEPTION(ConversionUtility::getNativeString(obj));
}

std::string ConversionUtility::getNativeString(v8::Local<v8::Value> js)
{
    if (!js->IsString())
    {
        throw "string";
    }

    return std::string(*Nan::Utf8String(js));
}

v8::Handle<v8::Value> ConversionUtility::toJsNumber(int32_t nativeValue)
{
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::New<v8::Integer>(nativeValue));
}

v8::Handle<v8::Value> ConversionUtility::toJsNumber(uint32_t nativeValue)
{
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::New<v8::Integer>(nativeValue));
}

v8::Handle<v8::Value> ConversionUtility::toJsNumber(uint16_t nativeValue)
{
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::New<v8::Integer>(nativeValue));
}

v8::Handle<v8::Value> ConversionUtility::toJsNumber(uint8_t nativeValue)
{
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::New<v8::Integer>(nativeValue));
}

v8::Handle<v8::Value> ConversionUtility::toJsNumber(double nativeValue)
{
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::New<v8::Number>(nativeValue));
}

v8::Handle<v8::Value> ConversionUtility::toJsBool(uint8_t nativeValue)
{
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::New<v8::Boolean>(nativeValue ? true : false));
}

v8::Handle<v8::Value> ConversionUtility::toJsValueArray(uint8_t *nativeData, uint16_t length)
{
    Nan::EscapableHandleScope scope;

    v8::Local<v8::Array> valueArray = Nan::New<v8::Array>(length);

    for (int i = 0; i < length; ++i)
    {
        valueArray->Set(i, ConversionUtility::toJsNumber(nativeData[i]));
    }

    return scope.Escape(valueArray);
}

v8::Handle<v8::Value> ConversionUtility::toJsString(const char *cString)
{
    return ConversionUtility::toJsString(cString, strlen(cString));
}

v8::Handle<v8::Value> ConversionUtility::toJsString(const char *cString, size_t length)
{
    Nan::EscapableHandleScope scope;
    auto name = static_cast<char*>(malloc(length + 1));
	assert(name != nullptr);

    memset(name, 0, length + 1); // Zero terminate the name
    memcpy(name, cString, length);

    v8::Local<v8::String> _name = Nan::New(name).ToLocalChecked();

    free(name);

    return scope.Escape(_name);
}

v8::Handle<v8::Value> ConversionUtility::toJsString(uint8_t *cString, size_t length)
{
    return ConversionUtility::toJsString(reinterpret_cast<const char *>(cString), length);
}


v8::Handle<v8::Value> ConversionUtility::toJsString(std::string string)
{
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::New<v8::String>(string).ToLocalChecked());
}

const char * ConversionUtility::valueToString(uint16_t value, name_map_t name_map, const char *defaultValue)
{
    name_map_it_t it = name_map.find(value);

    if (it == name_map.end())
    {
        return defaultValue;
    }

    return it->second;
}

v8::Handle<v8::Value> ConversionUtility::valueToJsString(uint16_t value, name_map_t name_map, v8::Handle<v8::Value> defaultValue)
{
    Nan::EscapableHandleScope scope;
    name_map_it_t it = name_map.find(value);

    if (it == name_map.end())
    {
        return defaultValue;
    }

    return scope.Escape(Nan::New<v8::String>(it->second).ToLocalChecked());
}

v8::Local<v8::Function> ConversionUtility::getCallbackFunction(v8::Local<v8::Object> js, const char *name)
{
    v8::Local<v8::Value> obj = Utility::Get(js, name);

    return ConversionUtility::getCallbackFunction(obj);
}

v8::Local<v8::Function> ConversionUtility::getCallbackFunction(v8::Local<v8::Value> js)
{
    Nan::EscapableHandleScope scope;
    if (!js->IsFunction())
    {
        throw "function";
    }
    return scope.Escape(js.As<v8::Function>());
}

uint8_t ConversionUtility::extractHexHelper(char text)
{
    if (text >= '0' && text <= '9')
    {
        return text - '0';
    }

    if (text >= 'a' && text <= 'f')
    {
        return text - 'a' + 10;
    }

    if (text >= 'A' && text <= 'F')
    {
        return text - 'A' + 10;
    }

    return 0xFF;
}

uint8_t *ConversionUtility::extractHex(v8::Local<v8::Value> js)
{
    v8::Local<v8::String> jsString = v8::Local<v8::String>::Cast(js);
    auto length = jsString->Length();
    auto cString = static_cast<char *>(malloc(sizeof(char) * (length + 1)));
    memset(cString, 0, length + 1);

    jsString->WriteUtf8(cString, length);

    auto size = (length / 2);

    auto retArray = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * size));
    memset(retArray, 0, size);

    for (auto i = 0, j = size - 1; i < length; i += 2, j--)
    {
        auto first = extractHexHelper(cString[i]);
        auto second = extractHexHelper(cString[i + 1]);

        if (first == 0xFF || second == 0xFF)
        {
            continue;
        }

        retArray[j] = (first << 4) + second;
    }

    return retArray;
}

v8::Handle<v8::Value> ConversionUtility::encodeHex(const char *text, int length)
{
    std::ostringstream encoded;
    encoded.flags(std::ios::uppercase);
    encoded.width(2);
    encoded.fill('0');

    for (auto i = length - 1; i >= 0; --i)
    {
        encoded << std::hex << (static_cast<int>(text[i]) & 0xFF);
    }

    return ConversionUtility::toJsString(encoded.str());
}

v8::Local<v8::Value> Utility::Get(v8::Local<v8::Object> jsobj, const char *name)
{
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::Get(jsobj, Nan::New(name).ToLocalChecked()).ToLocalChecked());
}

v8::Local<v8::Value> Utility::Get(v8::Local<v8::Object> jsobj, const int index)
{
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::Get(jsobj, index).ToLocalChecked());
}

void Utility::SetMethod(v8::Handle<v8::Object> target, const char *exportName, Nan::FunctionCallback function)
{
    Utility::Set(target,
        exportName,
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(function)).ToLocalChecked());
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, int32_t value)
{
    return Utility::Set(target, name, ConversionUtility::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, uint32_t value)
{
    return Utility::Set(target, name, ConversionUtility::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, int16_t value)
{
    return Utility::Set(target, name, ConversionUtility::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, uint16_t value)
{
    return Utility::Set(target, name, ConversionUtility::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, int8_t value)
{
    return Utility::Set(target, name, ConversionUtility::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, uint8_t value)
{
    return Utility::Set(target, name, ConversionUtility::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, bool value)
{
    return Utility::Set(target, name, ConversionUtility::toJsBool(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, double value)
{
    return Utility::Set(target, name, ConversionUtility::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, const char *value)
{
    return Utility::Set(target, name, ConversionUtility::toJsString(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, std::string value)
{
    return Utility::Set(target, name, ConversionUtility::toJsString(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, v8::Local<v8::Value> value)
{
    return Nan::Set(target, Nan::New(name).ToLocalChecked(), value).FromMaybe(false);
}

bool Utility::Has(v8::Handle<v8::Object> target, const char *name)
{
    return target->Has(Nan::New(name).ToLocalChecked());
}

bool Utility::Has(v8::Handle<v8::Object> target, const int index)
{
    return target->Has(Nan::New<v8::Integer>(index));
}

void Utility::SetReturnValue(Nan::NAN_METHOD_ARGS_TYPE info, v8::Local<v8::Object> value)
{
    info.GetReturnValue().Set(value);
}

bool Utility::IsObject(v8::Local<v8::Object> jsobj, const char *name)
{
    return Utility::Get(jsobj, name)->IsObject();
}

bool Utility::IsNull(v8::Local<v8::Object> jsobj, const char *name)
{
    return Utility::Get(jsobj, name)->IsNull();
}

bool Utility::IsNull(v8::Local<v8::Object> jsobj)
{
    if (Utility::Has(jsobj, "special_hack_null_object"))
    {
        return true;
    }
    if (!jsobj->IsObject())
    {
        return true;
    }
    return jsobj->IsNull();
}

bool Utility::IsBetween(const uint8_t value, const uint8_t min, const uint8_t max)
{
    if (value < min || value > max)
    {
        return false;
    }

    return true;
}

bool Utility::EnsureAsciiNumbers(uint8_t *value, const int length)
{
    for (int i = 0; i < length; ++i)
    {
        if (IsBetween(value[i], 0, 9))
        {
            value[i] = value[i] + '0';
        }
        else if (!IsBetween(value[i], '0', '9'))
        {
            return false;
        }
    }

    return true;
}

v8::Local<v8::Value> ErrorMessage::getErrorMessage(const int errorCode, const std::string customMessage)
{
    Nan::EscapableHandleScope scope;

    switch (errorCode)
    {
        case errorcodes::JsSuccess:
            return scope.Escape(Nan::Undefined());

        default:
        {
            std::ostringstream errorStringStream;
            errorStringStream << "Error occured when " << customMessage << ". "
                << "Errorcode: " << ConversionUtility::valueToString(errorCode, nrfjprog_js_err_map) << " (0x" << std::hex << errorCode << ")" << std::endl;

            v8::Local<v8::Value> error = Nan::Error(ConversionUtility::toJsString(errorStringStream.str())->ToString());
            v8::Local<v8::Object> errorObject = error.As<v8::Object>();

            Utility::Set(errorObject, "errno", errorCode);
            Utility::Set(errorObject, "errcode", ConversionUtility::valueToString(errorCode, nrfjprog_js_err_map));
            Utility::Set(errorObject, "erroperation", ConversionUtility::toJsString(customMessage));
            Utility::Set(errorObject, "errmsg", ConversionUtility::toJsString(errorStringStream.str()));

            return scope.Escape(error);
        }
    }
}

v8::Local<v8::String> ErrorMessage::getTypeErrorMessage(const int argumentNumber, const std::string message)
{
    std::ostringstream stream;

    switch (argumentNumber)
    {
        case 0:
            stream << "First";
            break;
        case 1:
            stream << "Second";
            break;
        case 2:
            stream << "Third";
            break;
        case 3:
            stream << "Fourth";
            break;
        case 4:
            stream << "Fifth";
            break;
        case 5:
            stream << "Sixth";
            break;
        case 6:
            stream << "Seventh";
            break;
        default:
            stream << "Unknown";
            break;
    }

    stream << " argument must be a " << message;

    return ConversionUtility::toJsString(stream.str())->ToString();
}

v8::Local<v8::String> ErrorMessage::getStructErrorMessage(const std::string name, const std::string message)
{
    std::ostringstream stream;

    stream << "Property: " << name << " Message: " << message;

    return ConversionUtility::toJsString(stream.str())->ToString();
}
