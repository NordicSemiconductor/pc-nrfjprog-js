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

#ifndef COMMON_H
#define COMMON_H

#include <nan.h>
#include <map>
#include <mutex>
#include <string>

#include "nrfjprog_common.h"

#define NAME_MAP_ENTRY(EXP) { EXP, ""#EXP"" }

#define BATON_CONSTRUCTOR(BatonType) BatonType(v8::Local<v8::Function> callback) : Baton(callback) {}
#define BATON_DESTRUCTOR(BatonType) ~BatonType()

#define METHOD_DEFINITIONS(MainName) \
    NAN_METHOD(MainName); \
    void MainName(uv_work_t *req); \
    void After##MainName(uv_work_t *req);

// Typedef of name to string with enum name, covers most cases
typedef std::map<uint16_t, const char*> name_map_t;
typedef std::map<uint16_t, const char*>::iterator name_map_it_t;

class ConversionUtility;

class Utility
{
public:
    static v8::Local<v8::Value> Get(v8::Local<v8::Object> jsobj, const char *name);
    static v8::Local<v8::Value> Get(v8::Local<v8::Object> jsobj, const int index);
    static void SetMethod(v8::Handle<v8::Object> target, const char *exportName, Nan::FunctionCallback function);

    static bool Set(v8::Handle<v8::Object> target, const char *name, int32_t value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, uint32_t value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, int16_t value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, uint16_t value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, int8_t value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, uint8_t value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, bool value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, double value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, const char *value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, std::string value);
    static bool Set(v8::Handle<v8::Object> target, const char *name, v8::Local<v8::Value> value);

    static bool Has(v8::Handle<v8::Object> target, const char *name);
    static bool Has(v8::Handle<v8::Object> target, const int index);

    static void SetReturnValue(Nan::NAN_METHOD_ARGS_TYPE info, v8::Local<v8::Object> value);

    static bool IsObject(v8::Local<v8::Object> jsobj, const char *name);
    static bool IsNull(v8::Local<v8::Object> jsobj, const char *name);
    static bool IsNull(v8::Local<v8::Object> jsobj);

    static bool IsBetween(const uint8_t value, const uint8_t min, const uint8_t max);
    static bool EnsureAsciiNumbers(uint8_t *value, const int length);
};

struct Baton {
public:
    explicit Baton(v8::Local<v8::Function> cb) {
        req = new uv_work_t();
        callback = new Nan::Callback(cb);
        req->data = static_cast<void*>(this);
    }

    ~Baton()
    {
        delete callback;
    }

    uv_work_t *req;
    Nan::Callback *callback;

    uint32_t result;
};

const std::string getCurrentTimeInMilliseconds();

uint16_t uint16_decode(const uint8_t *p_encoded_data);
uint32_t uint32_decode(const uint8_t *p_encoded_data);

uint16_t fromNameToValue(name_map_t names, const char *name);

template<typename NativeType>
class ConvUtil
{
public:
    static NativeType getNativeUnsigned(v8::Local<v8::Value> js)
    {
        if (!js->IsNumber())
        {
            throw "number";
        }

        return static_cast<NativeType>(js->ToUint32()->Uint32Value());
    }

    static NativeType getNativeSigned(v8::Local<v8::Value> js)
    {
        if (!js->IsNumber())
        {
            throw "number";
        }

        return static_cast<NativeType>(js->ToInt32()->Int32Value());
    }

    static NativeType getNativeFloat(v8::Local<v8::Value> js)
    {
        if (!js->IsNumber())
        {
            throw "number";
        }

        return static_cast<NativeType>(js->ToNumber()->NumberValue());
    }

    static NativeType getNativeBool(v8::Local<v8::Value> js)
    {
        if (!js->IsBoolean())
        {
            throw "bool";
        }

        return static_cast<NativeType>(js->ToBoolean()->BooleanValue());
    }

    static NativeType getNativeUnsigned(v8::Local<v8::Object> js, const char *name)
    {
        return getNativeUnsigned(js->Get(Nan::New(name).ToLocalChecked()));
    }

    static NativeType getNativeSigned(v8::Local<v8::Object> js, const char *name)
    {
        return getNativeSigned(js->Get(Nan::New(name).ToLocalChecked()));
    }

    static NativeType getNativeFloat(v8::Local<v8::Object> js, const char *name)
    {
        return getNativeFloat(js->Get(Nan::New(name).ToLocalChecked()));
    }

    static NativeType getNativeBool(v8::Local<v8::Object> js, const char *name)
    {
        return getNativeBool(js->Get(Nan::New(name).ToLocalChecked()));
    }
};

class ConversionUtility
{
public:
    static uint32_t     getNativeUint32(v8::Local<v8::Object>js, const char *name);
    static uint32_t     getNativeUint32(v8::Local<v8::Value> js);
    static uint16_t     getNativeUint16(v8::Local<v8::Object>js, const char *name);
    static uint16_t     getNativeUint16(v8::Local<v8::Value> js);
    static uint8_t      getNativeUint8(v8::Local<v8::Object>js, const char *name);
    static uint8_t      getNativeUint8(v8::Local<v8::Value> js);
    static int32_t      getNativeInt32(v8::Local<v8::Object>js, const char *name);
    static int32_t      getNativeInt32(v8::Local<v8::Value> js);
    static int16_t      getNativeInt16(v8::Local<v8::Object>js, const char *name);
    static int16_t      getNativeInt16(v8::Local<v8::Value> js);
    static int8_t       getNativeInt8(v8::Local<v8::Object>js, const char *name);
    static int8_t       getNativeInt8(v8::Local<v8::Value> js);
    static double       getNativeDouble(v8::Local<v8::Object>js, const char *name);
    static double       getNativeDouble(v8::Local<v8::Value> js);
    static uint8_t      getNativeBool(v8::Local<v8::Object>js, const char *name);
    static uint8_t      getNativeBool(v8::Local<v8::Value>js);
    static bool         getBool(v8::Local<v8::Object>js, const char *name);
    static bool         getBool(v8::Local<v8::Value>js);
    static uint8_t *    getNativePointerToUint8(v8::Local<v8::Object>js, const char *name);
    static uint8_t *    getNativePointerToUint8(v8::Local<v8::Value>js);
    static uint16_t *   getNativePointerToUint16(v8::Local<v8::Object>js, const char *name);
    static uint16_t *   getNativePointerToUint16(v8::Local<v8::Value>js);
    static v8::Local<v8::Object> getJsObject(v8::Local<v8::Object>js, const char *name);
    static v8::Local<v8::Object> getJsObject(v8::Local<v8::Value>js);
    static v8::Local<v8::Object> getJsObjectOrNull(v8::Local<v8::Object>js, const char *name);
    static v8::Local<v8::Object> getJsObjectOrNull(v8::Local<v8::Value>js);
    static uint16_t     stringToValue(name_map_t name_map, v8::Local<v8::Object> string, uint16_t defaultValue = -1);
    static std::string  getNativeString(v8::Local<v8::Object>js, const char *name);
    static std::string  getNativeString(v8::Local<v8::Value> js);

    static v8::Handle<v8::Value> toJsNumber(int32_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(uint32_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(uint16_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(uint8_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(double nativeValue);
    static v8::Handle<v8::Value> toJsBool(uint8_t nativeValue);
    static v8::Handle<v8::Value> toJsValueArray(uint8_t *nativeValue, uint16_t length);
    static v8::Handle<v8::Value> toJsString(const char *cString);
    static v8::Handle<v8::Value> toJsString(const char *cString, size_t length);
    static v8::Handle<v8::Value> toJsString(uint8_t *cString, size_t length);
    static v8::Handle<v8::Value> toJsString(std::string string);
    static const char *          valueToString(uint16_t value, name_map_t name_map, const char *defaultValue = "Unknown value");
    static v8::Handle<v8::Value> valueToJsString(uint16_t, name_map_t name_map, v8::Handle<v8::Value> defaultValue = Nan::New<v8::String>("Unknown value").ToLocalChecked());

    static v8::Local<v8::Function> getCallbackFunction(v8::Local<v8::Object> js, const char *name);
    static v8::Local<v8::Function> getCallbackFunction(v8::Local<v8::Value> js);

    static uint8_t extractHexHelper(char text);
    static uint8_t *extractHex(v8::Local<v8::Value> js);
    static v8::Handle<v8::Value> encodeHex(const char *text, int length);
};

class ErrorMessage
{
public:
    static v8::Local<v8::Value> getErrorMessage(const int errorCode, const std::string customMessage);
    static v8::Local<v8::String> getTypeErrorMessage(const int argumentNumber, const std::string message);
    static v8::Local<v8::String> getStructErrorMessage(const std::string name, const std::string message);
};

typedef enum errorcodes
{
    JsSuccess,
    CouldNotFindJlinkDLL,
    CouldNotFindJprogDLL,
    CouldNotLoadDLL,
    CouldNotOpenDevice,
    CouldNotConnectToDevice,
    CouldNotCallFunction,
    CouldNotErase,
    CouldNotProgram,
    CouldNotRead,
    CouldNotOpenHexFile,
    WrongMagicNumber
} errorcodes;

#endif // COMMON_H
