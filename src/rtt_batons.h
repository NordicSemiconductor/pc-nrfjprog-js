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

#include "rtt.h"
#include "rtt_helpers.h"
#include <memory>
#include <sstream>
#include <string>

class RTTBaton
{
  public:
    explicit RTTBaton(const std::string _name, const uint32_t _returnParameterCount)
        : returnParameterCount(_returnParameterCount)
        , name(_name)
        , result(JsSuccess)
        , lowlevelError(SUCCESS)
    {
        req       = std::make_unique<uv_work_t>();
        req->data = static_cast<void *>(this);
    }

    virtual ~RTTBaton()
    {
        req.reset();
        callback.reset();
    }

    virtual uint32_t returnParamterCount()
    {
        return returnParameterCount + 1;
    }

    virtual std::string toString()
    {
        return std::string();
    }

    const uint32_t returnParameterCount;
    const std::string name;

    uint32_t result;
    nrfjprogdll_err_t lowlevelError;

    std::unique_ptr<uv_work_t> req;
    std::unique_ptr<Nan::Callback> callback;

    std::chrono::high_resolution_clock::time_point functionStart;

    rtt_execute_function_t executeFunction;
    rtt_return_function_t returnFunction;

    static std::timed_mutex executionMutex;
};

std::timed_mutex RTTBaton::executionMutex;

class RTTStartBaton : public RTTBaton
{
  public:
    RTTStartBaton()
        : RTTBaton("start rtt", 2)
    {}
    std::string toString()
    {
        std::stringstream stream;

        stream << "Parameters:" << std::endl;
        stream << "Serialnumber: " << serialNumber << std::endl;
        stream << "Has Controlblock: " << (hasControlBlockLocation ? "true" : "false") << std::endl;
        stream << "Controlblock location: " << controlBlockLocation << std::endl;

        return stream.str();
    }

    uint32_t serialNumber;
    bool hasControlBlockLocation;
    uint32_t controlBlockLocation;

    uint32_t clockSpeed;
    device_family_t family;
    std::string jlinkarmlocation;

    std::vector<std::unique_ptr<ChannelInfo>> upChannelInfo;
    std::vector<std::unique_ptr<ChannelInfo>> downChannelInfo;
};

class RTTStopBaton : public RTTBaton
{
  public:
    RTTStopBaton()
        : RTTBaton("stop rtt", 0)
    {}
    std::string toString()
    {
        std::stringstream stream;

        stream << "No parameters";

        return stream.str();
    }
};

class RTTReadBaton : public RTTBaton
{
  public:
    RTTReadBaton()
        : RTTBaton("rtt read", 3)
    {}
    std::string toString()
    {
        std::stringstream stream;

        stream << "Parameters:" << std::endl;
        stream << "ChanneldIndex: " << channelIndex << std::endl;
        stream << "Length wanted: " << length << std::endl;

        return stream.str();
    }

    uint32_t channelIndex;
    uint32_t length;
    std::vector<char> data;
};

class RTTWriteBaton : public RTTBaton
{
  public:
    RTTWriteBaton()
        : RTTBaton("rtt write", 2)
    {}
    std::string toString()
    {
        std::stringstream stream;

        stream << "Parameters:" << std::endl;
        stream << "ChanneldIndex: " << channelIndex << std::endl;
        stream << "Length wanted: " << length << std::endl;
        stream << "Data" << data.data() << std::endl;

        return stream.str();
    }

    uint32_t channelIndex;
    uint32_t length;
    std::vector<char> data;
};

#endif
