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

#ifndef CONVERSION_H
#define CONVERSION_H

#include <nan.h>
#include <chrono>
#include "../common.h"

class Convert
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
    static char *       getNativePointerToChar(v8::Local<v8::Object>js, const char *name);
    static char *       getNativePointerToChar(v8::Local<v8::Value>js);
    static uint8_t *    getNativePointerToUint8(v8::Local<v8::Object>js, const char *name);
    static uint8_t *    getNativePointerToUint8(v8::Local<v8::Value>js);
    static uint16_t *   getNativePointerToUint16(v8::Local<v8::Object>js, const char *name);
    static uint16_t *   getNativePointerToUint16(v8::Local<v8::Value>js);
    static std::vector<char> getVectorForChar(v8::Local<v8::Object>js, const char *name);
    static std::vector<char> getVectorForChar(v8::Local<v8::Value> js);
    static std::vector<uint8_t> getVectorForUint8(v8::Local<v8::Object>js, const char *name);
    static std::vector<uint8_t> getVectorForUint8(v8::Local<v8::Value> js);
    static uint32_t     getLengthOfArray(v8::Local<v8::Object>js, const char *name);
    static uint32_t     getLengthOfArray(v8::Local<v8::Value>js);
    static v8::Local<v8::Object> getJsObject(v8::Local<v8::Object>js, const char *name);
    static v8::Local<v8::Object> getJsObject(v8::Local<v8::Value>js);
    static v8::Local<v8::Object> getJsObjectOrNull(v8::Local<v8::Object>js, const char *name);
    static v8::Local<v8::Object> getJsObjectOrNull(v8::Local<v8::Value>js);
    static uint16_t     stringToValue(name_map_t name_map, v8::Local<v8::Object> string, uint16_t defaultValue = -1);
    static std::string  getNativeString(v8::Local<v8::Object>js, const char *name);
    static std::string  getNativeString(v8::Local<v8::Value> js);

    static v8::Handle<v8::Value> toJsNumber(int32_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(uint32_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(int16_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(uint16_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(int8_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(uint8_t nativeValue);
    static v8::Handle<v8::Value> toJsNumber(double nativeValue);
    static v8::Handle<v8::Value> toJsBool(uint8_t nativeValue);
    static v8::Handle<v8::Value> toJsValueArray(uint8_t *nativeValue, uint32_t length);
    static v8::Handle<v8::Value> toJsString(const char *cString);
    static v8::Handle<v8::Value> toJsString(const char *cString, size_t length);
    static v8::Handle<v8::Value> toJsString(uint8_t *cString, size_t length);
    static v8::Handle<v8::Value> toJsString(std::string string);
    static const char *          valueToString(uint16_t value, name_map_t name_map, const char *defaultValue = "Unknown value");
    static v8::Handle<v8::Value> valueToJsString(uint16_t, name_map_t name_map, v8::Handle<v8::Value> defaultValue = Nan::New<v8::String>("Unknown value").ToLocalChecked());

    static v8::Handle<v8::Value> toTimeDifferenceUS(std::chrono::high_resolution_clock::time_point startTime, std::chrono::high_resolution_clock::time_point endTime);

    static v8::Local<v8::Function> getCallbackFunction(v8::Local<v8::Object> js, const char *name);
    static v8::Local<v8::Function> getCallbackFunction(v8::Local<v8::Value> js);

    static uint8_t extractHexHelper(char text);
    static uint8_t *extractHex(v8::Local<v8::Value> js);
    static v8::Handle<v8::Value> encodeHex(const char *text, int length);
};

#endif
