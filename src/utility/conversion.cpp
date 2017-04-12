#include "conversion.h"

#include <sstream>
#include <iostream>

#include "utility.h"

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

template<typename NativeType>
class ConvUtil
{
public:
    static NativeType getNativeUnsigned(v8::Local<v8::Value> js)
    {
        if (!js->IsNumber())
        {
            throw std::string("unsigned integer");
        }

        return static_cast<NativeType>(js->ToUint32()->Uint32Value());
    }

    static NativeType getNativeSigned(v8::Local<v8::Value> js)
    {
        if (!js->IsNumber())
        {
            throw std::string("signed integer");
        }

        return static_cast<NativeType>(js->ToInt32()->Int32Value());
    }

    static NativeType getNativeFloat(v8::Local<v8::Value> js)
    {
        if (!js->IsNumber())
        {
            throw std::string("float");
        }

        return static_cast<NativeType>(js->ToNumber()->NumberValue());
    }

    static NativeType getNativeBool(v8::Local<v8::Value> js)
    {
        if (!js->IsBoolean())
        {
            throw std::string("bool");
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
        throw std::string("array");
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
        throw std::string("object");
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

    throw std::string("object or null");
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
        throw std::string("string");
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
        throw std::string("function");
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
