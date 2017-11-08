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

#ifndef HIGHLEVEL_HELPERS_H
#define HIGHLEVEL_HELPERS_H

#include <nan.h>
#include "highlevelnrfjprogdll.h"
#include "highlevel_common.h"

class ProbeDetails
{
public:
    ProbeDetails(uint32_t _serial_number, device_info_t _device_info, probe_info_t _probe_info, library_info_t _library_info) :
        serial_number(_serial_number), device_info(_device_info), probe_info(_probe_info), library_info(_library_info)
    {}

    v8::Local<v8::Object> ToJs();

private:
    const uint32_t serial_number;
    const device_info_t device_info;
    const probe_info_t probe_info;
    const library_info_t library_info;
};

class ProbeInfo
{
  public:
    ProbeInfo(probe_info_t _probe_info) :
        probe_info(_probe_info)
    {
    }

    v8::Local<v8::Object> ToJs();

  private:
    const probe_info_t probe_info;
};

class DeviceInfo
{
public:
    DeviceInfo(device_info_t _device_info) :
        device_info(_device_info)
    {}

    v8::Local<v8::Object> ToJs();

private:
    const device_info_t device_info;
};

class LibraryInfo
{
public:
    LibraryInfo(library_info_t _library_info) :
        library_info(_library_info)
    {}

    v8::Local<v8::Object> ToJs();

private:
    const library_info_t library_info;
};

class EraseOptions
{
public:
    EraseOptions(v8::Local<v8::Object> obj);

    erase_action_t eraseMode;
    uint32_t startAddress;
    uint32_t endAddress;
};

class ReadToFileOptions
{
public:
    ReadToFileOptions(v8::Local<v8::Object> obj);

    read_options_t options;
};

class ProgramOptions
{
public:
    ProgramOptions(v8::Local<v8::Object> obj);

    program_options_t options;
    input_format_t inputFormat;
};

class VerifyOptions
{
public:
    VerifyOptions(v8::Local<v8::Object> obj);
};

#endif
