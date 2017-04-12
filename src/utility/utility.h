#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <nan.h>

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

#endif
