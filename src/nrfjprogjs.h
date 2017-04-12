/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in or with a processor manufactured by Nordic
 *   Semiconductor ASA, or in or with a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *   5. Any software provided in binary or object form under this license must not be
 *   reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __NRFJPROG_H__
#define __NRFJPROG_H__

#include <nan.h>
#include "common.h"
#include "dllfunc.h"
#include "osfiles.h"

#include "utility/errormessage.h"

class ProbeInfo
{
public:
    ProbeInfo(uint32_t serial_number, device_family_t family) :
        serial_number(serial_number), family(family)
    {}

    uint32_t serial_number;
    device_family_t family;

    v8::Local<v8::Object> ToJs();
};

class DebugProbe : public Nan::ObjectWrap
{
public:
    static NAN_MODULE_INIT(Init);

private:
    explicit DebugProbe();
    ~DebugProbe();

    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);

    // Sync methods

    // Async methods
    static NAN_METHOD(GetDllVersion); // Params: callback(error, dllversion)
    static NAN_METHOD(GetConnectedDevices); // Params: callback(error, connectedDevices)

    static NAN_METHOD(GetFamily); // Params: serialnumber, callback(error, family)
    static NAN_METHOD(Read); // Params: serialnumber, address, length, callback(error, family)

    static void CallFunction(Nan::NAN_METHOD_ARGS_TYPE info, parse_parameters_function_t parse, execute_function_t execute, return_function_t ret);
    static void DebugProbe::ExecuteFunction(uv_work_t *req);
    static void DebugProbe::ReturnFunction(uv_work_t *req);

    static errorcodes loadDll();
    static void unloadDll();

    static void init(v8::Local<v8::FunctionTemplate> tpl);

    static void closeBeforeExit();

    static void logCallback(const char * msg);

    static uint32_t emulatorSpeed;
    static DllFunctionPointersType dll_function;
    static char dll_path[COMMON_MAX_PATH];
    static char jlink_path[COMMON_MAX_PATH];

    static bool loaded;
    static bool connectedToDevice;
    static errorcodes finderror;

    static std::string logMessage;
};

#endif // __NRFJPROG_H__
