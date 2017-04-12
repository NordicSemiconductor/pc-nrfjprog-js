#ifndef __CONVERSION_H__
#define __CONVERSION_H__

#include <nan.h>
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

#endif
