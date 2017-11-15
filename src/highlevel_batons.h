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

#ifndef NRFJPROG_BATONS_H
#define NRFJPROG_BATONS_H

#include <memory>
#include "highlevel.h"
#include "highlevel_helpers.h"
#include "highlevel_common.h"

class Baton {
public:
    explicit Baton(const std::string _name, const uint32_t _returnParameterCount, const bool _mayHaveProgressCallback) :
        returnParameterCount(_returnParameterCount),
        name(_name),
        mayHaveProgressCallback(_mayHaveProgressCallback),
        serialNumber(0),
        result(JsSuccess),
        lowlevelError(SUCCESS)
    {
        req = std::make_unique<uv_work_t>();
        req->data = static_cast<void*>(this);
    }

    virtual ~Baton()
    {
        req.reset();
        callback.reset();
    }

    const uint32_t returnParameterCount;
    const std::string name;
    const bool mayHaveProgressCallback;

    uint32_t serialNumber;
    uint32_t result;
    nrfjprogdll_err_t lowlevelError;

    std::unique_ptr<uv_work_t> req;
    std::unique_ptr<Nan::Callback> callback;

    execute_function_t executeFunction;
    return_function_t returnFunction;
};

class GetDllVersionBaton : public Baton
{
public:
    GetDllVersionBaton() : Baton("get dll version", 1, false) {}
    uint32_t major;
    uint32_t minor;
    uint32_t revision;
};

class GetConnectedDevicesBaton : public Baton
{
public:
    GetConnectedDevicesBaton() : Baton("get connected devices", 1, false) {}
    std::vector<std::unique_ptr<ProbeDetails>> probes;
};

class GetDeviceInfoBaton : public Baton
{
public:
    GetDeviceInfoBaton() : Baton("get device info", 1, false) {}
    uint32_t serialNumber;
    device_info_t deviceInfo;
};

class GetProbeInfoBaton : public Baton
{
public:
    GetProbeInfoBaton() : Baton("get probe info", 1, false) {}
    probe_info_t probeInfo;
};

class GetLibraryInfoBaton : public Baton
{
public:
    GetLibraryInfoBaton() : Baton("get library info", 1, false) {}
    library_info_t libraryInfo;
};

class GetDeviceVersionBaton : public Baton
{
public:
    GetDeviceVersionBaton() : Baton("get device version", 1, false) {}
    uint32_t serialNumber;
    device_version_t deviceVersion;
};

class ReadBaton : public Baton
{
public:
    ReadBaton() : Baton("read", 1, false) {}

    uint32_t address;
    uint32_t length;
    std::vector<uint8_t> data;
};

class ReadU32Baton : public Baton
{
public:
    ReadU32Baton() : Baton("read u32", 1, false) {}
    uint32_t address;
    uint32_t length;
    uint32_t data;
};

class ProgramBaton : public Baton
{
public:
    ProgramBaton() : Baton("program", 0, true) {}
    std::string file;
    std::string filename;
    program_options_t options;
    input_format_t inputFormat;
};

class VerifyBaton : public Baton
{
public:
    VerifyBaton() : Baton("verify", 0, true) {}
    std::string filename;
};

class ReadToFileBaton : public Baton
{
public:
    ReadToFileBaton() : Baton("read to file", 0, true) {}
    std::string filename;
    read_options_t options;
};

class EraseBaton : public Baton
{
public:
    EraseBaton() : Baton("erase", 0, true) {}
    erase_action_t erase_mode;
    uint32_t start_address;
    uint32_t end_address;
};

class RecoverBaton : public Baton
{
public:
    RecoverBaton() : Baton("recover", 0, true) {}
};

class WriteBaton : public Baton
{
public:
    WriteBaton() : Baton("write", 0, false) {}
    uint32_t address;
    std::vector<uint8_t> data;
    uint32_t length;
};

class WriteU32Baton : public Baton
{
public:
    WriteU32Baton() : Baton("write u32", 0, false) {}
    uint32_t address;
    uint32_t data;
};

class OpenBaton : public Baton
{
public:
    OpenBaton() : Baton("open device long term", 0, false) {}
};

class CloseBaton : public Baton
{
public:
    CloseBaton() : Baton("close opened device", 0, false) {}
};

#endif
