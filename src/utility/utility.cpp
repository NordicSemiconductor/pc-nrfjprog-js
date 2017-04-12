#include "utility.h"
#include "conversion.h"

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
    return Utility::Set(target, name, Convert::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, uint32_t value)
{
    return Utility::Set(target, name, Convert::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, int16_t value)
{
    return Utility::Set(target, name, Convert::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, uint16_t value)
{
    return Utility::Set(target, name, Convert::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, int8_t value)
{
    return Utility::Set(target, name, Convert::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, uint8_t value)
{
    return Utility::Set(target, name, Convert::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, bool value)
{
    return Utility::Set(target, name, Convert::toJsBool(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, double value)
{
    return Utility::Set(target, name, Convert::toJsNumber(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, const char *value)
{
    return Utility::Set(target, name, Convert::toJsString(value));
}

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, std::string value)
{
    return Utility::Set(target, name, Convert::toJsString(value));
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
