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
#ifndef RTT_H
#define RTT_H

#include <nan.h>
#include "common.h"
#include "nrfjprogwrapper.h"
#include "osfiles.h"

#include "utility/errormessage.h"

#include <functional>
#include <chrono>
#include <memory>

class RTTBaton;
class RTTStartBaton;

typedef std::function<RTTBaton*(Nan::NAN_METHOD_ARGS_TYPE, int&)> rtt_parse_parameters_function_t;
typedef std::function<RTTErrorcodes_t(RTTBaton*)> rtt_execute_function_t;
typedef std::function<std::vector<v8::Local<v8::Value>>(RTTBaton*)> rtt_return_function_t;

class RTT : public Nan::ObjectWrap
{
public:
    static NAN_MODULE_INIT(Init);

    static void initConsts(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE tpl);

private:
    explicit RTT();
    ~RTT();

    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);

    static NAN_METHOD(Start); // Params: serialNumber, { location }, callback(error, down, up)
    static NAN_METHOD(Stop); // Params: callback(error)

    static NAN_METHOD(Read); // Params: channelIndex, callback(error, data, raw, time)
    static NAN_METHOD(Write); // Params: channelIndex, data, callback(error, writtenlength, time)

    static void CallFunction(Nan::NAN_METHOD_ARGS_TYPE info,
                            const rtt_parse_parameters_function_t parse,
                            const rtt_execute_function_t execute,
                            const rtt_return_function_t ret);
    static void ExecuteFunction(uv_work_t *req);
    static void ReturnFunction(uv_work_t *req);

    static void init(v8::Local<v8::FunctionTemplate> tpl);

    static RTTErrorcodes_t getDeviceInformation(RTTStartBaton *baton);
    static RTTErrorcodes_t waitForControlBlock(RTTStartBaton *baton);
    static RTTErrorcodes_t getChannelInformation(RTTStartBaton *baton);

    static bool isStarted();

    static void cleanup();

    static void log(const char * msg);
    static void resetLog();

    static std::string logMessage;
    static std::timed_mutex logMutex;
    static bool libraryLoaded;
    static std::chrono::high_resolution_clock::time_point rttStartTime;

    static nRFjprogLibraryFunctionPointersType libraryFunctions;
};

#endif
