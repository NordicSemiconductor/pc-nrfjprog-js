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
#ifndef __NRFJPROG_H__
#define __NRFJPROG_H__

#include <nan.h>
#include "common.h"
#include "dllfunc.h"
#include "osfiles.h"

#include "utility/errormessage.h"

class Baton;

typedef std::vector<v8::Local<v8::Value> > returnType;
typedef std::function<Baton*(Nan::NAN_METHOD_ARGS_TYPE, int&)> parse_parameters_function_t;
typedef std::function<nrfjprogdll_err_t(Baton*, Probe_handle_t)> execute_function_t;
typedef std::function<returnType(Baton*)> return_function_t;

class nRFjprog : public Nan::ObjectWrap
{
public:
    static NAN_MODULE_INIT(Init);

private:
    explicit nRFjprog();
    ~nRFjprog();

    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);

    // Sync methods

    // Async methods
    static NAN_METHOD(GetDllVersion); // Params: callback(error, dllversion)
    static NAN_METHOD(GetConnectedDevices); // Params: callback(error, connectedDevices)

    static NAN_METHOD(GetDeviceInfo); // Params: serialnumber, callback(error, family)

    static NAN_METHOD(Read);        // Params: serialnumber, address, length, callback(error, data)
    static NAN_METHOD(ReadU32);     // Params: serialnumber, address, callback(error, data)

    static NAN_METHOD(Program);     // Params: serialnumber, filename, options {verify, chip_erase_mode, qspi_erase_mode, reset}, callback(progress), callback(error)
    static NAN_METHOD(ReadToFile);  // Params: serialnumber, filename, options {readram, readcode, readuicr, readqspi}, callback(progress), callback(error)
    static NAN_METHOD(Verify);      // Params: serialnumber, filename, callback(progress), callback(error)
    static NAN_METHOD(Erase);       // Params: serialnumber, options {erase_mode, start_address, end_address}, callback(progress), callback(error)

    static NAN_METHOD(Recover);     // Params: serialnumber, callback(progress), callback(error)

    static NAN_METHOD(Write);       // Params: serialnumber, address, dataarray, callback(error)
    static NAN_METHOD(WriteU32);    // Params: serialnumber, address, data, callback(error)

    static void CallFunction(Nan::NAN_METHOD_ARGS_TYPE info,
                             const parse_parameters_function_t parse,
                             const execute_function_t execute,
                             const return_function_t ret,
                             const bool hasSerialNumber);
    static void ExecuteFunction(uv_work_t *req);
    static void ReturnFunction(uv_work_t *req);

    static errorcode_t loadDll();
    static void unloadDll();

    static void init(v8::Local<v8::FunctionTemplate> tpl);

    static void logCallback(const char * msg);
    static void log(std::string msg);

    static void progressCallback(const char * process);
    static Nan::Callback *jsProgressCallback;
    static void sendProgress(uv_async_t *handle);
    static uv_async_t *progressEvent;

    static DllFunctionPointersType dll_function;
    static char dll_path[COMMON_MAX_PATH];

    static bool loaded;
    static bool connectedToDevice;
    static errorcode_t finderror;

    static std::string logMessage;
};

#endif // __NRFJPROG_H__
