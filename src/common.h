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

#ifndef COMMON_H
#define COMMON_H

#include <nan.h>
#include <map>
#include <mutex>
#include <string>

#include "nrfjprog_common.h"
#include <functional>
#include "dllfunc.h"

struct Baton;

typedef std::vector<v8::Local<v8::Value> > returnType;
typedef std::function<Baton*(Nan::NAN_METHOD_ARGS_TYPE, int&)> parse_parameters_function_t;
typedef std::function<nrfjprogdll_err_t(Baton*, Probe_handle_t)> execute_function_t;
typedef std::function<returnType(Baton*)> return_function_t;

#define NAME_MAP_ENTRY(EXP) { EXP, ""#EXP"" }

#define BATON_CONSTRUCTOR(BatonType) BatonType(uint32_t serialNumber, uint32_t parameterCount, std::string name) : Baton(serialNumber, parameterCount, name) {}
#define BATON_DESTRUCTOR(BatonType) ~BatonType()

#define METHOD_DEFINITIONS(MainName) \
    NAN_METHOD(MainName); \
    void MainName(uv_work_t *req); \
    void After##MainName(uv_work_t *req);

// Typedef of name to string with enum name, covers most cases
typedef std::map<uint16_t, const char*> name_map_t;
typedef std::map<uint16_t, const char*>::iterator name_map_it_t;

struct Baton {
public:
    explicit Baton(const uint32_t _serialNumber, const uint32_t _returnParameterCount, std::string _name) {
        req = new uv_work_t();
        //callback = new Nan::Callback(cb);
        req->data = static_cast<void*>(this);
        serialNumber = _serialNumber;
        returnParameterCount = _returnParameterCount;
        name = _name;
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
};

const std::string getCurrentTimeInMilliseconds();

uint16_t uint16_decode(const uint8_t *p_encoded_data);
uint32_t uint32_decode(const uint8_t *p_encoded_data);

uint16_t fromNameToValue(name_map_t names, const char *name);

#endif // COMMON_H
