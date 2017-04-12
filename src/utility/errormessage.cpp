#include "errormessage.h"

#include <sstream>
#include <iostream>

#include "conversion.h"
#include "utility.h"

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
