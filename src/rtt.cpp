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

#include "utility/conversion.h"
#include "utility/errormessage.h"
#include "utility/utility.h"

#include <sstream>
#include <iostream>

Nan::Persistent<v8::Function> RTT::constructor;
std::string RTT::logMessage;
nRFjprogDllFunctionPointersType RTT::dll_function;

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
{}

RTT::~RTT()
{}

void RTT::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "start", Start);
    Nan::SetPrototypeMethod(tpl, "stop", Stop);

    Nan::SetPrototypeMethod(tpl, "read", Read);
}

void RTT::CallFunction(Nan::NAN_METHOD_ARGS_TYPE info, rtt_parse_parameters_function_t parse, rtt_execute_function_t execute, rtt_return_function_t ret)
{
    // This is a check that there exists a parse- and execute function, both of which are
    // needed to parse arguments and execute the function.
    // If this shows up in production, it is due to missing functions in the relevant NAN_METHOD defining the functions.
    if (parse == nullptr ||
        execute == nullptr)
    {
        auto message = ErrorMessage::getErrorMessage(1, std::string("One or more of the parse, or execute functions is missing for this function"));
        Nan::ThrowError(message);
        return;
    }

    logMessage.clear();

    auto argumentCount = 0;
    RTTBaton *baton = nullptr;
/*
    if (jsProgressCallback != nullptr)
    {
        delete jsProgressCallback;
        jsProgressCallback = nullptr;
    }
*/
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
/*
        if (jsProgressCallback != nullptr)
        {
            delete jsProgressCallback;
            jsProgressCallback = nullptr;
        }
*/
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);

        return;
    }

    // This is a check that there exists a returnfunction when there are more returns
    // than just err. If this shows up in production, it is due to missing return function
    if (ret == nullptr &&
        baton->returnParameterCount > 0)
    {
        auto message = ErrorMessage::getErrorMessage(1, std::string("The return function has more than one parameter and is required for this function, but is missing"));
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

    nrfjprogdll_err_t executeError = baton->executeFunction(baton);

    if (executeError != SUCCESS)
    {
        baton->result = errorcode_t::CouldNotCallFunction;
        baton->lowlevelError = executeError;
    }
}

void RTT::ReturnFunction(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<RTTBaton *>(req->data);
    //TODO: Create an arrary of correct size instead of a way to large one.
    v8::Local<v8::Value> argv[10];//baton->returnParameterCount + 1];

    if (baton->result != errorcode_t::JsSuccess)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, baton->name, logMessage, baton->lowlevelError);

        for (uint32_t i = 0; i < baton->returnParameterCount; i++)
        {
            argv[i + 1] = Nan::Undefined();
        }
    }
    else
    {
        argv[0] = Nan::Undefined();

        if (baton->returnFunction != nullptr)
        {
            std::vector<v8::Local<v8::Value> > vector = baton->returnFunction(baton);

            for (uint32_t i = 0; i < vector.size(); ++i)
            {
                argv[i + 1] = vector[i];
            }
        }
    }
/*
    if (jsProgressCallback != nullptr)
    {
        delete jsProgressCallback;
        jsProgressCallback = nullptr;
    }
*/
    baton->callback->Call(baton->returnParameterCount + 1, argv);

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

    rtt_execute_function_t e = [&] (RTTBaton *b) -> nrfjprogdll_err_t {
        auto baton = static_cast<RTTStartBaton*>(b);

        loadnRFjprogFunctions("C:\\Program Files (x86)\\Nordic Semiconductor\\nrf5x\\bin\\nrfjprog.dll", &dll_function);

        std::string jlinkarmlocation = "C:\\Program Files (x86)\\SEGGER\\JLink_V620f\\JLinkARM.dll";

        const nrfjprogdll_err_t openStatus = dll_function.open_dll(jlinkarmlocation.c_str(), 0, NRF52_FAMILY);

        if (openStatus != SUCCESS)
        {
            return OUT_OF_MEMORY;
        }

        //log("Serialnumber: " + baton->serialNumber);
        const nrfjprogdll_err_t connectToEmuStatus = dll_function.connect_to_emu_with_snr(baton->serialNumber, 4000);

        if (connectToEmuStatus != SUCCESS)
        {
            log("connectToEmuStatus " + connectToEmuStatus);
            return INVALID_OPERATION;
        }
        const nrfjprogdll_err_t conentToDeviceStatus = dll_function.connect_to_device();

        if (conentToDeviceStatus != SUCCESS)
        {
            return INVALID_PARAMETER;
        }
        const nrfjprogdll_err_t startStatus = dll_function.rtt_start();

        if (startStatus != SUCCESS)
        {
            return INVALID_DEVICE_FOR_OPERATION;
        }

        bool controlBlockFound = false;

        for(int i = 0; i < 100; ++i) {
            dll_function.rtt_is_control_block_found(&controlBlockFound);

            if (controlBlockFound) {
                return SUCCESS;
            }
        }

        const nrfjprogdll_err_t channelCountStatus = dll_function.rtt_read_channel_count(&baton->downChannelNumber, &baton->upChannelNumber);

        if (startStatus != SUCCESS)
        {
            return EMULATOR_NOT_CONNECTED;
        }

        log("Oops");
        return SUCCESS;
    };

    rtt_return_function_t r = [&] (RTTBaton *b) -> returnType {
        auto baton = static_cast<RTTStartBaton*>(b);

        returnType vector;

        vector.push_back(Convert::toJsNumber(baton->downChannelNumber));
        vector.push_back(Convert::toJsNumber(baton->upChannelNumber));

        return vector;
    };

    CallFunction(info, p, e, r);
}

NAN_METHOD(RTT::Stop)
{
    rtt_parse_parameters_function_t p = [&] (Nan::NAN_METHOD_ARGS_TYPE parameters, int &argumentCount) -> RTTBaton* {
        return new RTTStopBaton();
    };

    rtt_execute_function_t e = [&] (RTTBaton *b) -> nrfjprogdll_err_t {
        auto baton = static_cast<RTTStopBaton*>(b);

        dll_function.rtt_stop();
        dll_function.disconnect_from_device();
        dll_function.disconnect_from_emu();
        dll_function.close_dll();

        return SUCCESS;
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

    rtt_execute_function_t e = [&] (RTTBaton *b) -> nrfjprogdll_err_t {
        auto baton = static_cast<RTTReadBaton*>(b);

        baton->data = new char[baton->length];
        uint32_t readLength = 0;
        dll_function.rtt_read(baton->channelIndex, baton->data, baton->length, &readLength);

        baton->length = readLength;

        return SUCCESS;
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

        baton->data = (char *)Convert::getNativePointerToUint8(info[argumentCount]);
        baton->length = Convert::getLengthOfArray(info[argumentCount]);
        argumentCount++;

        return baton;
    };

    rtt_execute_function_t e = [&] (RTTBaton *b) -> nrfjprogdll_err_t {
        auto baton = static_cast<RTTWriteBaton*>(b);

        uint32_t writeLength = 0;

        dll_function.rtt_write(baton->channelIndex, baton->data, baton->length, &writeLength);

        baton->length = writeLength;

        return SUCCESS;
    };

    CallFunction(info, p, e, nullptr);
}
