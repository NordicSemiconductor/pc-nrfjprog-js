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

#ifndef __NRFJPROG_BATONS_H__
#define __NRFJPROG_BATONS_H__

#include <memory>
#include "nrfjprogjs.h"
#include "nrfjprog_helpers.h"

#define BATON_CONSTRUCTOR(BatonType, returnParameterCount) BatonType(std::string name) : Baton(returnParameterCount, name) {}
#define BATON_DESTRUCTOR(BatonType) ~BatonType()

class Baton {
public:
    explicit Baton(const uint32_t _returnParameterCount, std::string _name) {
        req = new uv_work_t();
        req->data = static_cast<void*>(this);
        returnParameterCount = _returnParameterCount;
        name = _name;
        lowlevelError = SUCCESS;
    }

    ~Baton()
    {
        delete callback;
    }

    uv_work_t *req;
    Nan::Callback *callback;
    uint32_t serialNumber;
    uint32_t returnParameterCount;

    execute_function_t executeFunction;
    return_function_t returnFunction;
    std::string name;

    uint32_t result;

    nrfjprogdll_err_t lowlevelError;
};

class GetDllVersionBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(GetDllVersionBaton, 1);
    uint32_t major;
    uint32_t minor;
    uint32_t revision;
};

class GetConnectedDevicesBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(GetConnectedDevicesBaton, 1);
    std::vector<ProbeInfo *> probes;
};

class GetFamilyBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(GetFamilyBaton, 1);
    uint32_t serialNumber;
    device_family_t family;
};

class GetDeviceVersionBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(GetDeviceVersionBaton, 1);
    uint32_t serialNumber;
    device_version_t deviceVersion;
};

class ReadBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(ReadBaton, 1);
    BATON_DESTRUCTOR(ReadBaton)
    {
        if (data != nullptr)
        {
            delete[] data;
            data = nullptr;
        }
    }

    uint32_t address;
    uint32_t length;
    uint8_t *data;
};

class ReadU32Baton : public Baton
{
public:
    BATON_CONSTRUCTOR(ReadU32Baton, 1);
    uint32_t address;
    uint32_t length;
    uint32_t data;
};

class ProgramBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(ProgramBaton, 0);
    std::string filename;
    program_options_t options;
};

class VerifyBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(VerifyBaton, 0);
    std::string filename;
};

class ReadToFileBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(ReadToFileBaton, 0);
    std::string filename;
    read_options_t options;
};

class EraseBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(EraseBaton, 0);
    erase_mode_t erase_mode;
    uint32_t start_address;
    uint32_t end_address;
};

class WriteBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(WriteBaton, 0);
    uint32_t address;
    uint8_t *data;
    uint32_t length;
};

class WriteU32Baton : public Baton
{
public:
    BATON_CONSTRUCTOR(WriteU32Baton, 0);
    uint32_t address;
    uint32_t data;
};

#endif
