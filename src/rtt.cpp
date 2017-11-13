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

#include "rtt.h"

#include <vector>

#include "highlevel_common.h"
#include "rtt_batons.h"
#include "rtt_helpers.h"
#include "highlevelwrapper.h"

#include "utility/conversion.h"
#include "utility/errormessage.h"
#include "utility/utility.h"

#include <sstream>
#include <iostream>

Nan::Persistent<v8::Function> RTT::constructor;
std::string RTT::logMessage;
nRFjprogDllFunctionPointersType RTT::dll_function;
bool RTT::libraryLoaded;
std::chrono::high_resolution_clock::time_point RTT::rttStartTime;

#define RETURN_ERROR_ON_FAIL(function, error) do { \
    const nrfjprogdll_err_t status = (function); \
    \
    if (status != SUCCESS) \
    { \
        cleanup(); \
        baton->lowlevelError = status; \
        return error; \
    } \
} while(0);

#define TIMED_RETURN_ERROR_ON_FAIL(function, error, baton) do { \
    baton->functionStart = std::chrono::high_resolution_clock::now(); \
    const nrfjprogdll_err_t status = (function); \
    baton->functionEnd = std::chrono::high_resolution_clock::now(); \
    \
    if (status != SUCCESS) \
    { \
        cleanup(); \
        baton->lowlevelError = status; \
        return error; \
    } \
} while(0);

NAN_MODULE_INIT(RTT::Init)
{
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RTT").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    init(tpl);

    constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("RTT").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(RTT::New)
{
    if (info.IsConstructCall())
    {
        auto obj = new RTT();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New(constructor);
        info.GetReturnValue().Set(cons->NewInstance());
    }
}

RTT::RTT()
{
    libraryLoaded = false;
}

RTT::~RTT()
{}

void RTT::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "start", Start);
    Nan::SetPrototypeMethod(tpl, "stop", Stop);

    Nan::SetPrototypeMethod(tpl, "read", Read);
    Nan::SetPrototypeMethod(tpl, "write", Write);
}

void RTT::CallFunction(Nan::NAN_METHOD_ARGS_TYPE info, rtt_parse_parameters_function_t parse, rtt_execute_function_t execute, rtt_return_function_t ret)
{
    // This is a check that there exists a parse- and execute function, both of which are
    // needed to parse arguments and execute the function.
    // If this shows up in production, it is due to missing functions in the relevant NAN_METHOD defining the functions.
    if (parse == nullptr ||
        execute == nullptr)
    {
        auto message = ErrorMessage::getErrorMessage(1, rtt_err_map, std::string("One or more of the parse, or execute functions is missing for this function"));
        Nan::ThrowError(message);
        return;
    }

    logMessage.clear();

    auto argumentCount = 0;
    RTTBaton *baton = nullptr;

    try
    {
        baton = parse(info, argumentCount);

        v8::Local<v8::Function> callback = Convert::getCallbackFunction(info[argumentCount]);
        baton->callback = new Nan::Callback(callback);
        argumentCount++;

        if (info.Length() > argumentCount)
        {
            argumentCount = CUSTOM_ARGUMENT_PARSE_ERROR;
            std::ostringstream errorStringStream;
            errorStringStream << "Too many parameters. The function " << baton->name << " do not take " << info.Length() << " parameters.";
            throw errorStringStream.str();
        }
    }
    catch (std::string error)
    {
        if (baton != nullptr)
        {
            delete baton;
        }

        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);

        return;
    }

    // This is a check that there exists a returnfunction when there are more returns
    // than just err. If this shows up in production, it is due to missing return function
    if (ret == nullptr &&
        baton->returnParameterCount > 0)
    {
        auto message = ErrorMessage::getErrorMessage(1, rtt_err_map, std::string("The return function has more than one parameter and is required for this function, but is missing"));
        Nan::ThrowError(message);
        return;
    }

    log("===============================================\n");
    log("Start of ");
    log(baton->name);
    log("\n");
    log("===============================================\n");


    baton->executeFunction = execute;
    baton->returnFunction = ret;

    uv_queue_work(uv_default_loop(), baton->req, ExecuteFunction, reinterpret_cast<uv_after_work_cb>(ReturnFunction));
}

void RTT::ExecuteFunction(uv_work_t *req)
{
    auto baton = static_cast<RTTBaton *>(req->data);

    RTTErrorcodes_t executeError = baton->executeFunction(baton);

    if (executeError != RTTSuccess)
    {
        baton->result = executeError;
    }
}

void RTT::ReturnFunction(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<RTTBaton *>(req->data);
    std::vector<v8::Local<v8::Value> > argv;

    argv.push_back(ErrorMessage::getErrorMessage(baton->result, rtt_err_map, baton->name, logMessage, baton->lowlevelError));

    if (baton->result != errorcode_t::JsSuccess)
    {
        for (uint32_t i = 0; i < baton->returnParameterCount; i++)
        {
            argv.push_back(Nan::Undefined());
        }
    }
    else
    {
        if (baton->returnFunction != nullptr)
        {
            std::vector<v8::Local<v8::Value> > vector = baton->returnFunction(baton);

            argv.insert(argv.end(), vector.begin(), vector.end());
        }
    }

    v8::Local<v8::Object> timeObj = Nan::New<v8::Object>();

    Utility::Set(timeObj, "timeSinceEpoch", Convert::toTimeDifference(std::chrono::high_resolution_clock::time_point(), baton->functionStart));
    Utility::Set(timeObj, "timeSinceRTTStart", Convert::toTimeDifference(rttStartTime, baton->functionStart));
    Utility::Set(timeObj, "functioncallTime", Convert::toTimeDifference(baton->functionStart, baton->functionEnd));

    argv.push_back(timeObj);

    baton->callback->Call(baton->returnParameterCount + 2, argv.data());

    delete baton;
}

void RTT::log(std::string msg)
{
    logMessage = logMessage.append(msg);
}

NAN_METHOD(RTT::Start)
{
    rtt_parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> RTTBaton* {
        auto baton = new RTTStartBaton();

        baton->serialNumber = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        v8::Local<v8::Object> startOptions = Convert::getJsObject(parameters[argumentCount]);
        argumentCount++;

        return baton;
    };

    rtt_execute_function_t e = [&] (RTTBaton *b) -> RTTErrorcodes_t {
        auto baton = static_cast<RTTStartBaton*>(b);

        DllFunctionPointersType highLevelFunctions;

        const nrfjprogdll_err_t loadHighlevelStatus = (nrfjprogdll_err_t)loadHighLevelFunctions(&highLevelFunctions);

        if (loadHighlevelStatus != SUCCESS)
        {
            baton->lowlevelError = loadHighlevelStatus;
            return RTTCouldNotOpenHighlevelLibrary;
        }

        const nrfjprogdll_err_t openHighlevelStatus = highLevelFunctions.dll_open(nullptr, nullptr, nullptr);

        if (openHighlevelStatus != SUCCESS)
        {
            releaseHighLevel();
            baton->lowlevelError = openHighlevelStatus;
            return RTTCouldNotOpenHighlevelLibrary;
        }

        Probe_handle_t probe;
        const nrfjprogdll_err_t initProbeStatus = highLevelFunctions.probe_init(&probe, baton->serialNumber, nullptr);

        if (initProbeStatus != SUCCESS)
        {
            highLevelFunctions.dll_close();
            releaseHighLevel();
            baton->lowlevelError = initProbeStatus;
            return RTTCouldNotGetDeviceInformation;
        }

        probe_info_t probeInfo;
        device_info_t deviceInfo;
        library_info_t libraryInfo;

        RETURN_ERROR_ON_FAIL(highLevelFunctions.get_probe_info(probe, &probeInfo), RTTCouldNotGetDeviceInformation);
        RETURN_ERROR_ON_FAIL(highLevelFunctions.get_device_info(probe, &deviceInfo), RTTCouldNotGetDeviceInformation);
        RETURN_ERROR_ON_FAIL(highLevelFunctions.get_library_info(probe, &libraryInfo), RTTCouldNotGetDeviceInformation);

        RETURN_ERROR_ON_FAIL(highLevelFunctions.reset(probe, RESET_SYSTEM), RTTCouldNotGetDeviceInformation);
        RETURN_ERROR_ON_FAIL(highLevelFunctions.probe_uninit(&probe), RTTCouldNotGetDeviceInformation);
        highLevelFunctions.dll_close();

        releaseHighLevel();

        uint32_t clockSpeed = probeInfo.clockspeed_khz;
        device_family_t family = deviceInfo.device_family;
        std::string jlinkarmlocation = libraryInfo.file_path;

        RETURN_ERROR_ON_FAIL((nrfjprogdll_err_t)loadnRFjprogFunctions(&dll_function), RTTCouldNotLoadnRFjprogLibrary);

        libraryLoaded = true;

        RETURN_ERROR_ON_FAIL(dll_function.open_dll(jlinkarmlocation.c_str(), 0, family), RTTCouldNotOpennRFjprogLibrary);

        RETURN_ERROR_ON_FAIL(dll_function.connect_to_emu_with_snr(baton->serialNumber, clockSpeed), RTTCouldNotConnectToDevice);
        RETURN_ERROR_ON_FAIL(dll_function.connect_to_device(), RTTCouldNotConnectToDevice);

        rttStartTime = std::chrono::high_resolution_clock::now();
        RETURN_ERROR_ON_FAIL(dll_function.rtt_start(), RTTCouldNotStartRTT);

        bool controlBlockFound = false;

        for(int i = 0; i < 100000; ++i) {
            RETURN_ERROR_ON_FAIL(dll_function.rtt_is_control_block_found(&controlBlockFound), RTTCouldNotCallFunction);

            if (controlBlockFound) {
                log("Found control block");
                break;
            }
        }

        uint32_t downChannelNumber;
        uint32_t upChannelNumber;

        RETURN_ERROR_ON_FAIL(dll_function.rtt_read_channel_count(&downChannelNumber, &upChannelNumber), RTTCouldNotGetChannelInformation);

        for(uint32_t i = 0; i < downChannelNumber; ++i)
        {
            char channelName[32];
            uint32_t channelSize;
            RETURN_ERROR_ON_FAIL(dll_function.rtt_read_channel_info(i, DOWN_DIRECTION, channelName, 32, &channelSize), RTTCouldNotGetChannelInformation);

            std::string name(channelName);
            baton->downChannelInfo.push_back(new ChannelInfo(i, name, channelSize));
        }

        for(uint32_t i = 0; i < upChannelNumber; ++i)
        {
            char channelName[32];
            uint32_t channelSize;
            RETURN_ERROR_ON_FAIL(dll_function.rtt_read_channel_info(i, UP_DIRECTION, channelName, 32, &channelSize), RTTCouldNotGetChannelInformation);

            std::string name(channelName);
            baton->upChannelInfo.push_back(new ChannelInfo(i, name, channelSize));
        }

        baton->functionEnd = std::chrono::high_resolution_clock::now();
        baton->functionStart = rttStartTime;

        return RTTSuccess;
    };

    rtt_return_function_t r = [&] (RTTBaton *b) -> returnType {
        auto baton = static_cast<RTTStartBaton*>(b);

        returnType vector;

        v8::Local<v8::Array> downChannelInfo = Nan::New<v8::Array>();
        int i = 0;
        for (auto element : baton->downChannelInfo)
        {
            Nan::Set(downChannelInfo, Convert::toJsNumber(i), element->ToJs());
            i++;
        }

        vector.push_back(downChannelInfo);

        v8::Local<v8::Array> upChannelInfo = Nan::New<v8::Array>();
        i = 0;
        for (auto element : baton->upChannelInfo)
        {
            Nan::Set(upChannelInfo, Convert::toJsNumber(i), element->ToJs());
            i++;
        }

        vector.push_back(upChannelInfo);

        return vector;
    };

    CallFunction(info, p, e, r);
}

void RTT::cleanup()
{
    if (!libraryLoaded)
    {
        return;
    }

    bool started;

    dll_function.is_rtt_started(&started);

    if (started)
    {
        dll_function.rtt_stop();
    }

    bool connected;

    dll_function.is_connected_to_device(&connected);

    if (connected)
    {
        dll_function.disconnect_from_emu();
    }

    dll_function.close_dll();

    releasenRFjprog();

    libraryLoaded = false;
}

NAN_METHOD(RTT::Stop)
{
    rtt_parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> RTTBaton* {
        return new RTTStopBaton();
    };


    rtt_execute_function_t e = [&] (RTTBaton *b) -> RTTErrorcodes_t {
        auto baton = static_cast<RTTStopBaton*>(b);

        baton->functionStart = std::chrono::high_resolution_clock::now();
        RETURN_ERROR_ON_FAIL(dll_function.rtt_stop(), RTTCouldNotCallFunction);
        RETURN_ERROR_ON_FAIL(dll_function.disconnect_from_emu(), RTTCouldNotCallFunction);
        dll_function.close_dll();
        baton->functionEnd = std::chrono::high_resolution_clock::now();

        releasenRFjprog();
        libraryLoaded = false;

        return RTTSuccess;
    };

    CallFunction(info, p, e, nullptr);
}

NAN_METHOD(RTT::Read)
{
    rtt_parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> RTTBaton* {
        auto baton = new RTTReadBaton();

        baton->channelIndex = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        baton->length = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        return baton;
    };

    rtt_execute_function_t e = [&] (RTTBaton *b) -> RTTErrorcodes_t {
        auto baton = static_cast<RTTReadBaton*>(b);

        baton->data = new char[baton->length];
        uint32_t readLength = 0;

        TIMED_RETURN_ERROR_ON_FAIL(dll_function.rtt_read(baton->channelIndex, baton->data, baton->length, &readLength), RTTCouldNotCallFunction, baton);

        baton->length = readLength;

        return RTTSuccess;
    };

    rtt_return_function_t r = [&] (RTTBaton *b) -> returnType {
        auto baton = static_cast<RTTReadBaton*>(b);

        returnType vector;

        vector.push_back(Convert::toJsString(baton->data, baton->length));
        vector.push_back(Convert::toJsValueArray((uint8_t *)baton->data, baton->length));

        return vector;
    };

    CallFunction(info, p, e, r);
}

NAN_METHOD(RTT::Write)
{
    rtt_parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> RTTBaton* {
        auto baton = new RTTWriteBaton();

        baton->channelIndex = Convert::getNativeUint32(info[argumentCount]);
        argumentCount++;

        if (info[argumentCount]->IsString())
        {
            baton->data = Convert::getNativePointerToChar(info[argumentCount]);
            baton->length = Convert::getNativeString(info[argumentCount]).length();
        }
        else
        {
            baton->data = (char *)Convert::getNativePointerToUint8(info[argumentCount]);
            baton->length = Convert::getLengthOfArray(info[argumentCount]);
        }
        argumentCount++;

        return baton;
    };

    rtt_execute_function_t e = [&] (RTTBaton *b) -> RTTErrorcodes_t {
        auto baton = static_cast<RTTWriteBaton*>(b);

        uint32_t writeLength = 0;

        TIMED_RETURN_ERROR_ON_FAIL(dll_function.rtt_write(baton->channelIndex, baton->data, baton->length, &writeLength), RTTCouldNotCallFunction, baton);

        baton->length = writeLength;

        return RTTSuccess;
    };

    rtt_return_function_t r = [&] (RTTBaton *b) -> returnType {
        auto baton = static_cast<RTTReadBaton*>(b);

        returnType vector;

        vector.push_back(Convert::toJsNumber(baton->length));

        return vector;
    };


    CallFunction(info, p, e, r);
}
