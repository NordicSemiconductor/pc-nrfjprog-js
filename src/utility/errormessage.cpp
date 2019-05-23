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

#include "errormessage.h"

#include <iostream>
#include <sstream>

#include "../highlevel_common.h"
#include "conversion.h"
#include "utility.h"

v8::Local<v8::Value> ErrorMessage::getErrorMessage(const int errorCode,
                                                   const name_map_t &errorcodeMapper,
                                                   const std::string &customMessage)
{
    return getErrorMessage(errorCode, errorcodeMapper, customMessage, std::string(),
                           static_cast<nrfjprogdll_err_t>(0));
}

v8::Local<v8::Value> ErrorMessage::getErrorMessage(const int errorCode,
                                                   const name_map_t &errorcodeMapper,
                                                   const std::string &customMessage,
                                                   const std::string &logmessage,
                                                   const nrfjprogdll_err_t &lowlevelError)
{
    Nan::EscapableHandleScope scope;

    if (errorCode == 0)
    {
        return scope.Escape(Nan::Undefined());
    }

    std::string errorcodeString(Convert::valueToString(errorCode, errorcodeMapper));

    std::ostringstream errorStringStream;
    errorStringStream << "Error occured when " << customMessage << ". "
                      << "Errorcode: " << errorcodeString << " (0x" << std::hex << errorCode << ")"
                      << std::endl;

    std::string lowLevelMessage(Convert::valueToString(lowlevelError, nrfjprogdll_err_map));

    if (lowlevelError != SUCCESS)
    {
        errorStringStream << "Lowlevel error: " << lowLevelMessage << " (" << lowlevelError << ")"
                          << std::endl;
    }

    v8::Local<v8::Value> error =
        Nan::Error(Convert::toJsString(errorStringStream.str())->ToString());
    v8::Local<v8::Object> errorObject = error.As<v8::Object>();

    Utility::Set(errorObject, "errno", Convert::toJsNumber(errorCode));
    Utility::Set(errorObject, "errcode", Convert::toJsString(errorcodeString));
    Utility::Set(errorObject, "erroperation", Convert::toJsString(customMessage));
    Utility::Set(errorObject, "errmsg", Convert::toJsString(errorStringStream.str()));
    Utility::Set(errorObject, "lowlevelErrorNo", Convert::toJsNumber(lowlevelError));
    Utility::Set(errorObject, "lowlevelError", Convert::toJsString(lowLevelMessage));
    Utility::Set(errorObject, "log", Convert::toJsString(logmessage));

    return scope.Escape(error);
}

v8::Local<v8::String> ErrorMessage::getTypeErrorMessage(const int argumentNumber,
                                                        const std::string &message)
{
    static name_map_t argumentCountMap = {{0, "First"}, {1, "Second"}, {2, "Third"},  {3, "Fourth"},
                                          {4, "Fifth"}, {5, "Sixth"},  {6, "Seventh"}};

    std::ostringstream stream;

    if (argumentNumber != CUSTOM_ARGUMENT_PARSE_ERROR)
    {
        stream << Convert::valueToString(argumentNumber, argumentCountMap, "Unknown")
               << " argument must be a " << message;
    }
    else
    {
        stream << message;
    }

    return Convert::toJsString(stream.str())->ToString();
}

v8::Local<v8::String> ErrorMessage::getStructErrorMessage(const std::string &name,
                                                          const std::string &message)
{
    std::ostringstream stream;

    stream << "Property: " << name << " Message: " << message;

    return Convert::toJsString(stream.str())->ToString();
}
