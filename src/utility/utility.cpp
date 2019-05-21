/* Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
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

void Utility::SetMethod(v8::Handle<v8::Object> target, const char *exportName,
                        Nan::FunctionCallback function)
{
    Utility::Set(target, exportName,
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

bool Utility::Set(v8::Handle<v8::Object> target, const char *name, const std::string &value)
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
    return !(value < min || value > max);
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
