#ifndef __ERRORMESSAGE_H__
#define __ERRORMESSAGE_H__

#include <nan.h>
#include "../common.h"

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
    CouldNotOpenDLL,
    CouldNotConnectToDevice,
    CouldNotCallFunction,
    CouldNotErase,
    CouldNotProgram,
    CouldNotRead,
    CouldNotOpenHexFile,
    WrongMagicNumber
} errorcodes;

static name_map_t nrfjprog_js_err_map = {
    { errorcodes::JsSuccess, "Success" },
    { errorcodes::CouldNotFindJlinkDLL, "CouldNotFindJlinkDLL" },
    { errorcodes::CouldNotFindJprogDLL, "CouldNotFindJprogDLL" },
    { errorcodes::CouldNotLoadDLL, "CouldNotLoadDLL" },
    { errorcodes::CouldNotOpenDLL, "CouldNotOpenDLL" },
    { errorcodes::CouldNotOpenDevice, "CouldNotOpenDevice" },
    { errorcodes::CouldNotConnectToDevice, "CouldNotConnectToDevice" },
    { errorcodes::CouldNotCallFunction, "CouldNotCallFunction" },
    { errorcodes::CouldNotErase, "CouldNotErase" },
    { errorcodes::CouldNotProgram, "CouldNotProgram" },
    { errorcodes::CouldNotRead, "CouldNotRead" },
    { errorcodes::CouldNotOpenHexFile, "CouldNotOpenHexFile" },
    { errorcodes::WrongMagicNumber, "WrongMagicNumber" }
};

#endif
