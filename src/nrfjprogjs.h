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

#define MAX_SERIAL_NUMBERS 100

#define NRFJPROGJS_METHOD_DEFINITIONS(MainName) \
    static NAN_METHOD(MainName); \
    static void MainName(uv_work_t *req); \
    static void After##MainName(uv_work_t *req);

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
    NRFJPROGJS_METHOD_DEFINITIONS(Program); // Params: Serialnumber, family, file, callback
    NRFJPROGJS_METHOD_DEFINITIONS(GetSerialnumbers); // Params: None, callback
    NRFJPROGJS_METHOD_DEFINITIONS(GetVersion); // Params: Serialnumber, family, callback
    NRFJPROGJS_METHOD_DEFINITIONS(ReadAddress); // Params: Serialnumber, family, address, length, callback

    static void loadDll();
    static void unloadDll();

    static device_family_t openDll(device_family_t family, const uint32_t serialnumber);

    static void init(v8::Local<v8::FunctionTemplate> tpl);

    static nrfjprogdll_err_t correctFamily(const uint32_t serialnumber);

    static void closeBeforeExit();

    static uint32_t emulatorSpeed;
    static DllFunctionPointersType dll_function;
    static char dll_path[COMMON_MAX_PATH];
    static char jlink_path[COMMON_MAX_PATH];

    static bool loaded;
    static bool connectedToDevice;
    static int error;
    static int finderror;
    static uint32_t versionMagicNumber;
};

class ConnectBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(ConnectBaton);
};

class DisconnectBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(DisconnectBaton);
};

class ProgramBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(ProgramBaton);
    uint32_t serialnumber;
    device_family_t family;
    std::string filename;
    std::map<device_family_t, std::string> filenameMap;
    bool useProvidedFamily;
    bool filecontent;
};

class GetSerialnumbersBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(GetSerialnumbersBaton);
    std::vector<ProbeInfo*> probes;
};

class GetVersionBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(GetVersionBaton);
    uint32_t serialnumber;
    device_family_t family;
    uint8_t versionData[24];
};

class ReadAddressBaton : public Baton
{
public:
    BATON_CONSTRUCTOR(ReadAddressBaton);
    uint32_t serialnumber;
    device_family_t family;
    uint32_t address;
    uint32_t length;
    uint8_t *data;
};

#endif // __NRFJPROG_H__
