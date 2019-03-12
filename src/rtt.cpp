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

#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include "DllCommonDefinitions.h"
#include "highlevel_common.h"
#include "highlevelwrapper.h"
#include "rtt_batons.h"
#include "rtt_helpers.h"

#include "utility/conversion.h"
#include "utility/errormessage.h"
#include "utility/utility.h"

struct RTTStaticPrivate
{
    std::string logMessage;
    std::timed_mutex logMutex;
    bool libraryLoaded{false};
    std::chrono::high_resolution_clock::time_point rttStartTime{};
    nRFjprogLibraryFunctionPointersType libraryFunctions{};

    static inline Nan::Persistent<v8::Function> &constructor()
    {
        static Nan::Persistent<v8::Function> my_constructor;
        return my_constructor;
    }
};

static RTTStaticPrivate *pRTTStatic = nullptr;

#define RETURN_ERROR_ON_FAIL(function, error)                                                      \
    do                                                                                             \
    {                                                                                              \
        const nrfjprogdll_err_t status = (function);                                               \
                                                                                                   \
        if (status != SUCCESS)                                                                     \
        {                                                                                          \
            cleanup();                                                                             \
            baton->lowlevelError = status;                                                         \
            return error;                                                                          \
        }                                                                                          \
    } while (0);

NAN_MODULE_INIT(RTT::Init)
{
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("RTT").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    init(tpl);

    RTTStaticPrivate::constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("RTT").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(RTT::New)
{
    if (info.IsConstructCall())
    {
        (new RTT())->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        const int argc                  = 1;
        v8::Local<v8::Value> argv[argc] = {info[0]};
        v8::Local<v8::Function> cons    = Nan::New(RTTStaticPrivate::constructor());
        info.GetReturnValue().Set(
            Nan::NewInstance(cons, argc, static_cast<v8::Local<v8::Value> *>(argv))
                .ToLocalChecked());
    }
}

RTT::RTT()
{
    static RTTStaticPrivate rttStatic;
    pRTTStatic = &rttStatic;
    resetLog();
}

RTT::~RTT() = default;

void RTT::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "start", Start);
    Nan::SetPrototypeMethod(tpl, "stop", Stop);

    Nan::SetPrototypeMethod(tpl, "read", Read);
    Nan::SetPrototypeMethod(tpl, "write", Write);
}

void RTT::initConsts(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
    NODE_DEFINE_CONSTANT(target, RTTSuccess); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotLoadHighlevelLibrary); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotOpenHighlevelLibrary); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotGetDeviceInformation); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotLoadnRFjprogLibrary); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotOpennRFjprogLibrary); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotConnectToDevice); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotStartRTT); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotFindControlBlock); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotGetChannelInformation); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotCallFunction); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTNotInitialized); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RTTCouldNotExecuteDueToLoad); // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, UP_DIRECTION); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, DOWN_DIRECTION); // NOLINT(hicpp-signed-bitwise)
}

void RTT::CallFunction(Nan::NAN_METHOD_ARGS_TYPE info, const rtt_parse_parameters_function_t &parse,
                       const rtt_execute_function_t &execute, const rtt_return_function_t &ret)
{
    // This is a check that there exists a parse- and execute function, both of which are
    // needed to parse arguments and execute the function.
    // If this shows up in production, it is due to missing functions in the relevant NAN_METHOD
    // defining the functions.
    if (parse == nullptr || execute == nullptr)
    {
        auto message = ErrorMessage::getErrorMessage(
            1, rtt_err_map,
            std::string(
                "One or more of the parse, or execute functions is missing for this function"));
        Nan::ThrowError(message);
        return;
    }

    resetLog();

    auto argumentCount              = 0;
    std::unique_ptr<RTTBaton> baton = nullptr;

    try
    {
        baton.reset(parse(info, argumentCount));

        v8::Local<v8::Function> callback = Convert::getCallbackFunction(info[argumentCount]);
        baton->callback                  = std::make_unique<Nan::Callback>(callback);
        ++argumentCount;

        if (info.Length() > argumentCount)
        {
            argumentCount = CUSTOM_ARGUMENT_PARSE_ERROR;
            std::ostringstream errorStringStream;
            errorStringStream << "Too many parameters. The function " << baton->name
                              << " does not take " << info.Length() << " parameters.";
            throw std::runtime_error(errorStringStream.str());
        }
    }
    catch (const std::runtime_error &error)
    {
        baton = nullptr;

        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error.what());
        Nan::ThrowTypeError(message);

        return;
    }

    // This is a check that there exists a returnfunction when there are more returns
    // than just err. If this shows up in production, it is due to missing return function
    if (ret == nullptr && baton->returnParameterCount > 0)
    {
        auto message = ErrorMessage::getErrorMessage(
            1, rtt_err_map,
            std::string("The return function has more than one parameter and is required for this "
                        "function, but is missing"));
        Nan::ThrowError(message);
        return;
    }

    baton->executeFunction = execute;
    baton->returnFunction  = ret;

    uv_queue_work(uv_default_loop(), baton.release()->req.get(), ExecuteFunction,
                  reinterpret_cast<uv_after_work_cb>(ReturnFunction));
}

void RTT::ExecuteFunction(uv_work_t *req)
{
    auto baton = static_cast<RTTBaton *>(req->data);

    std::unique_lock<std::timed_mutex> lock(RTTBaton::executionMutex, std::defer_lock);

    if (!lock.try_lock_for(std::chrono::seconds(10)))
    {
        baton->result = RTTCouldNotExecuteDueToLoad;
        return;
    }

    baton->result = baton->executeFunction(baton);

    lock.unlock();
}

void RTT::ReturnFunction(uv_work_t *req)
{
    Nan::HandleScope scope;

    std::unique_ptr<RTTBaton> baton(static_cast<RTTBaton *>(req->data));
    std::vector<v8::Local<v8::Value>> argv;

    std::string msg;

    {
        std::unique_lock<std::timed_mutex> lock(pRTTStatic->logMutex, std::defer_lock);

        if (lock.try_lock_for(std::chrono::seconds(10)))
        {
            msg = pRTTStatic->logMessage;
        }
    }

    argv.emplace_back(ErrorMessage::getErrorMessage(baton->result, rtt_err_map, baton->name, msg,
                                                    baton->lowlevelError));

    if (baton->result != errorcode_t::JsSuccess)
    {
        for (uint32_t i = 0; i < baton->returnParameterCount; i++)
        {
            argv.emplace_back(Nan::Undefined());
        }
    }
    else
    {
        if (baton->returnFunction != nullptr)
        {
            std::vector<v8::Local<v8::Value>> vector = baton->returnFunction(baton.get());

            argv.insert(argv.end(), vector.begin(), vector.end());
        }
    }

    Nan::AsyncResource resource("pc-nrfjprog-js:callback");
    baton->callback->Call(baton->returnParamterCount(), argv.data(), &resource);
}

void RTT::resetLog()
{
    std::unique_lock<std::timed_mutex> lock(pRTTStatic->logMutex, std::defer_lock);

    if (!lock.try_lock_for(std::chrono::seconds(10)))
    {
        return;
    }

    pRTTStatic->logMessage.clear();
}

void RTT::log(const char *msg)
{
    log(std::string(msg));
}

void RTT::log(const std::string &msg)
{
    std::unique_lock<std::timed_mutex> lock(pRTTStatic->logMutex, std::defer_lock);

    if (!lock.try_lock_for(std::chrono::seconds(10)))
    {
        return;
    }

    pRTTStatic->logMessage.append(msg);
}

bool RTT::isStarted()
{
    if (!pRTTStatic->libraryLoaded)
    {
        return false;
    }

    bool started;
    pRTTStatic->libraryFunctions.is_rtt_started(&started);

    return started;
}

RTTErrorcodes_t RTT::getDeviceInformation(RTTStartBaton *baton)
{
    LibraryFunctionPointersType highLevelFunctions{};

    const auto loadHighlevelStatus =
        static_cast<nrfjprogdll_err_t>(loadHighLevelFunctions(&highLevelFunctions));

    if (loadHighlevelStatus != SUCCESS)
    {
        baton->lowlevelError = loadHighlevelStatus;
        return RTTCouldNotOpenHighlevelLibrary;
    }

    const nrfjprogdll_err_t openHighlevelStatus =
        highLevelFunctions.dll_open(nullptr, &RTT::log, nullptr);

    if (openHighlevelStatus != SUCCESS)
    {
        releaseHighLevel();
        baton->lowlevelError = openHighlevelStatus;
        return RTTCouldNotOpenHighlevelLibrary;
    }

    Probe_handle_t probe;
    const nrfjprogdll_err_t initProbeStatus =
        highLevelFunctions.probe_init(&probe, baton->serialNumber, nullptr);

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

    RETURN_ERROR_ON_FAIL(highLevelFunctions.get_probe_info(probe, &probeInfo),
                         RTTCouldNotGetDeviceInformation);
    RETURN_ERROR_ON_FAIL(highLevelFunctions.get_device_info(probe, &deviceInfo),
                         RTTCouldNotGetDeviceInformation);
    RETURN_ERROR_ON_FAIL(highLevelFunctions.get_library_info(probe, &libraryInfo),
                         RTTCouldNotGetDeviceInformation);

    RETURN_ERROR_ON_FAIL(highLevelFunctions.reset(probe, RESET_SYSTEM),
                         RTTCouldNotGetDeviceInformation);
    RETURN_ERROR_ON_FAIL(highLevelFunctions.probe_uninit(&probe), RTTCouldNotGetDeviceInformation);
    highLevelFunctions.dll_close();

    releaseHighLevel();

    baton->clockSpeed = probeInfo.clockspeed_khz;
    baton->family     = deviceInfo.device_family;
    baton->jlinkarmlocation.assign(static_cast<const char *>(libraryInfo.file_path));

    return RTTSuccess;
}

RTTErrorcodes_t RTT::waitForControlBlock(RTTStartBaton *baton)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        bool controlBlockFound = false;
        RETURN_ERROR_ON_FAIL(
            pRTTStatic->libraryFunctions.rtt_is_control_block_found(&controlBlockFound),
            RTTCouldNotCallFunction);

        if (controlBlockFound)
        {
            return RTTSuccess;
        }

        auto attemptedStartupTime = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(attemptedStartupTime -
                                                             pRTTStatic->rttStartTime)
                .count() > 5)
        {
            cleanup();
            return RTTCouldNotFindControlBlock;
        }
    }

    return RTTSuccess;
}

RTTErrorcodes_t RTT::getChannelInformation(RTTStartBaton *baton)
{
    uint32_t downChannelNumber;
    uint32_t upChannelNumber;

    RETURN_ERROR_ON_FAIL(
        pRTTStatic->libraryFunctions.rtt_read_channel_count(&downChannelNumber, &upChannelNumber),
        RTTCouldNotGetChannelInformation);

    for (uint32_t i = 0; i < downChannelNumber; ++i)
    {
        char channelName[32];
        char *pChannelName = static_cast<char *>(channelName);
        uint32_t channelSize;
        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.rtt_read_channel_info(
                                 i, DOWN_DIRECTION, pChannelName, 32, &channelSize),
                             RTTCouldNotGetChannelInformation);

        std::string name(pChannelName);
        baton->downChannelInfo.emplace_back(
            std::make_unique<ChannelInfo>(i, DOWN_DIRECTION, name, channelSize));
    }

    for (uint32_t i = 0; i < upChannelNumber; ++i)
    {
        char channelName[32];
        char *pChannelName = static_cast<char *>(channelName);
        uint32_t channelSize;
        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.rtt_read_channel_info(
                                 i, UP_DIRECTION, pChannelName, 32, &channelSize),
                             RTTCouldNotGetChannelInformation);

        std::string name(pChannelName);
        baton->upChannelInfo.emplace_back(
            std::make_unique<ChannelInfo>(i, UP_DIRECTION, name, channelSize));
    }

    return RTTSuccess;
}

NAN_METHOD(RTT::Start)
{
    rtt_parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters,
                                            int &argumentCount) -> RTTBaton * {
        auto baton = std::make_unique<RTTStartBaton>();

        baton->serialNumber = Convert::getNativeUint32(info[argumentCount]);
        ++argumentCount;

        v8::Local<v8::Object> startOptions = Convert::getJsObject(parameters[argumentCount]);
        StartOptions options(startOptions);
        baton->hasControlBlockLocation = options.hasControlBlockLocation;
        baton->controlBlockLocation    = options.controlBlockLocation;
        ++argumentCount;

        return baton.release();
    };

    rtt_execute_function_t e = [&](RTTBaton *b) -> RTTErrorcodes_t {
        auto baton = dynamic_cast<RTTStartBaton *>(b);

        const RTTErrorcodes_t deviceInformationStatus = getDeviceInformation(baton);

        if (deviceInformationStatus != RTTSuccess)
        {
            return deviceInformationStatus;
        }

        RETURN_ERROR_ON_FAIL(
            (nrfjprogdll_err_t)loadnRFjprogFunctions(&pRTTStatic->libraryFunctions),
            RTTCouldNotLoadnRFjprogLibrary);

        pRTTStatic->libraryLoaded = true;

        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.open_dll(baton->jlinkarmlocation.c_str(),
                                                                   &RTT::log, baton->family),
                             RTTCouldNotOpennRFjprogLibrary);

        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.connect_to_emu_with_snr(
                                 baton->serialNumber, baton->clockSpeed),
                             RTTCouldNotConnectToDevice);
        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.connect_to_device(),
                             RTTCouldNotConnectToDevice);

        if (baton->hasControlBlockLocation)
        {
            RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.rtt_set_control_block_address(
                                     baton->controlBlockLocation),
                                 RTTCouldNotFindControlBlock);
        }

        pRTTStatic->rttStartTime = std::chrono::high_resolution_clock::now();
        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.rtt_start(), RTTCouldNotStartRTT);

        const RTTErrorcodes_t waitStatus = waitForControlBlock(baton);

        if (waitStatus != RTTSuccess)
        {
            return waitStatus;
        }

        const RTTErrorcodes_t channelInformationStatus = getChannelInformation(baton);

        return channelInformationStatus;
    };

    rtt_return_function_t r = [&](RTTBaton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<RTTStartBaton *>(b);

        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Array> downChannelInfo = Nan::New<v8::Array>();
        int i                                = 0;
        for (auto &element : baton->downChannelInfo)
        {
            Nan::Set(downChannelInfo, Convert::toJsNumber(i), element->ToJs());
            ++i;
        }

        returnData.emplace_back(downChannelInfo);

        v8::Local<v8::Array> upChannelInfo = Nan::New<v8::Array>();
        i                                  = 0;
        for (auto &element : baton->upChannelInfo)
        {
            Nan::Set(upChannelInfo, Convert::toJsNumber(i), element->ToJs());
            ++i;
        }

        returnData.emplace_back(upChannelInfo);

        return returnData;
    };

    CallFunction(info, p, e, r);
}

void RTT::cleanup()
{
    if (!pRTTStatic->libraryLoaded)
    {
        return;
    }

    bool started;

    pRTTStatic->libraryFunctions.is_rtt_started(&started);

    if (started)
    {
        pRTTStatic->libraryFunctions.rtt_stop();
    }

    bool connected;

    pRTTStatic->libraryFunctions.is_connected_to_device(&connected);

    if (connected)
    {
        pRTTStatic->libraryFunctions.disconnect_from_emu();
    }

    pRTTStatic->libraryFunctions.close_dll();

    releasenRFjprog();

    pRTTStatic->libraryLoaded = false;
}

NAN_METHOD(RTT::Stop)
{
    rtt_parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters,
                                            int &argumentCount) -> RTTBaton * {
        return new RTTStopBaton();
    };

    rtt_execute_function_t e = [&](RTTBaton *b) -> RTTErrorcodes_t {
        auto baton = dynamic_cast<RTTStopBaton *>(b);

        if (!isStarted())
        {
            return RTTNotInitialized;
        }

        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.rtt_stop(), RTTCouldNotCallFunction);
        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.disconnect_from_emu(),
                             RTTCouldNotCallFunction);
        pRTTStatic->libraryFunctions.close_dll();

        releasenRFjprog();
        pRTTStatic->libraryLoaded = false;

        return RTTSuccess;
    };

    CallFunction(info, p, e, nullptr);
}

NAN_METHOD(RTT::Read)
{
    rtt_parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters,
                                            int &argumentCount) -> RTTBaton * {
        auto baton = std::make_unique<RTTReadBaton>();

        baton->channelIndex = Convert::getNativeUint32(info[argumentCount]);
        ++argumentCount;

        baton->length = Convert::getNativeUint32(info[argumentCount]);
        ++argumentCount;

        return baton.release();
    };

    rtt_execute_function_t e = [&](RTTBaton *b) -> RTTErrorcodes_t {
        auto baton = dynamic_cast<RTTReadBaton *>(b);

        if (!isStarted())
        {
            return RTTNotInitialized;
        }

        baton->data.resize(baton->length, 0);
        uint32_t readLength = 0;

        baton->functionStart = std::chrono::high_resolution_clock::now();

        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.rtt_read(baton->channelIndex,
                                                                   baton->data.data(),
                                                                   baton->length, &readLength),
                             RTTCouldNotCallFunction);

        baton->length = readLength;

        return RTTSuccess;
    };

    rtt_return_function_t r = [&](RTTBaton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<RTTReadBaton *>(b);

        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(Convert::toJsString(baton->data.data(), baton->length));
        returnData.emplace_back(Convert::toJsValueArray(
            reinterpret_cast<uint8_t *>(baton->data.data()), baton->length));
        returnData.emplace_back(
            Convert::toTimeDifferenceUS(pRTTStatic->rttStartTime, baton->functionStart));

        return returnData;
    };

    CallFunction(info, p, e, r);
}

NAN_METHOD(RTT::Write)
{
    rtt_parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters,
                                            int &argumentCount) -> RTTBaton * {
        auto baton = std::make_unique<RTTWriteBaton>();

        baton->channelIndex = Convert::getNativeUint32(info[argumentCount]);
        ++argumentCount;

        if (info[argumentCount]->IsString())
        {
            baton->data   = Convert::getVectorForChar(info[argumentCount]);
            baton->length = Convert::getNativeString(info[argumentCount]).length();
        }
        else
        {
            std::vector<uint8_t> tempData = Convert::getVectorForUint8(info[argumentCount]);
            baton->data                   = std::vector<char>(tempData.begin(), tempData.end());
            baton->length                 = Convert::getLengthOfArray(info[argumentCount]);
        }
        ++argumentCount;

        return baton.release();
    };

    rtt_execute_function_t e = [&](RTTBaton *b) -> RTTErrorcodes_t {
        auto baton = dynamic_cast<RTTWriteBaton *>(b);

        if (!isStarted())
        {
            return RTTNotInitialized;
        }

        uint32_t writeLength = 0;

        baton->functionStart = std::chrono::high_resolution_clock::now();
        RETURN_ERROR_ON_FAIL(pRTTStatic->libraryFunctions.rtt_write(baton->channelIndex,
                                                                    baton->data.data(),
                                                                    baton->length, &writeLength),
                             RTTCouldNotCallFunction);

        baton->length = writeLength;

        return RTTSuccess;
    };

    rtt_return_function_t r = [&](RTTBaton *b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<RTTReadBaton *>(b);

        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(Convert::toJsNumber(baton->length));
        returnData.emplace_back(
            Convert::toTimeDifferenceUS(pRTTStatic->rttStartTime, baton->functionStart));

        return returnData;
    };

    CallFunction(info, p, e, r);
}
