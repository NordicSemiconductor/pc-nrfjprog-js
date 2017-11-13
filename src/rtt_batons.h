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

#ifndef RTT_BATONS_H
#define RTT_BATONS_H

#include <memory>
#include "rtt.h"
#include "rtt_helpers.h"

#define RTTBATON_CONSTRUCTOR(BatonType, name, returnParameterCount) BatonType() : RTTBaton(returnParameterCount, name) {}
#define RTTBATON_DESTRUCTOR(BatonType) ~BatonType()

class RTTBaton {
public:
    explicit RTTBaton(const uint32_t _returnParameterCount, const std::string _name) :
        returnParameterCount(_returnParameterCount),
        name(_name),
        result(JsSuccess),
        lowlevelError(SUCCESS)
    {
        req = new uv_work_t();
        req->data = static_cast<void*>(this);
        callback = nullptr;
    }

    virtual ~RTTBaton()
    {
        delete req;

        if (callback != nullptr)
        {
            delete callback;
            callback = nullptr;
        }
    }

    virtual uint32_t returnParamterCount()
    {
        return returnParameterCount + 1;
    }

    const uint32_t returnParameterCount;
    const std::string name;

    uint32_t result;
    nrfjprogdll_err_t lowlevelError;

    uv_work_t *req;
    Nan::Callback *callback;

    std::chrono::high_resolution_clock::time_point functionStart;

    rtt_execute_function_t executeFunction;
    rtt_return_function_t returnFunction;
};

class RTTStartBaton : public RTTBaton
{
public:
    RTTBATON_CONSTRUCTOR(RTTStartBaton, "start rtt", 2);
    uint32_t serialNumber;

    std::vector<ChannelInfo *> upChannelInfo;
    std::vector<ChannelInfo *> downChannelInfo;
};

class RTTStopBaton : public RTTBaton
{
public:
    RTTBATON_CONSTRUCTOR(RTTStopBaton, "stop rtt", 0);
};

class RTTReadBaton : public RTTBaton
{
public:
    RTTBATON_CONSTRUCTOR(RTTReadBaton, "rtt read", 3);
    RTTReadBaton::~RTTReadBaton() {
        delete[] data;
    }

    uint32_t channelIndex;
    uint32_t length;
    char *data;
};

class RTTWriteBaton : public RTTBaton
{
public:
    RTTBATON_CONSTRUCTOR(RTTWriteBaton, "rtt write", 2);
    RTTWriteBaton::~RTTWriteBaton() {
        delete[] data;
    }

    uint32_t channelIndex;
    uint32_t length;
    char *data;
};


#endif
