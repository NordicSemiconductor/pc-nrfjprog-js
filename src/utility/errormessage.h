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

#ifndef __ERRORMESSAGE_H__
#define __ERRORMESSAGE_H__

#include <nan.h>
#include "../common.h"

class ErrorMessage
{
public:
    static v8::Local<v8::Value> getErrorMessage(const int errorCode, const std::string customMessage);
    static v8::Local<v8::String> getTypeErrorMessage(const int argumentNumber, const std::string message);
    static v8::Local<v8::String> getStructErrorMessage(const std::string name, const std::string message);
};

typedef enum errorcodes
{
    JsSuccess,
    CouldNotFindJlinkDLL,
    CouldNotFindJprogDLL,
    CouldNotLoadDLL,
    CouldNotOpenDevice,
    CouldNotResetDevice,
    CouldNotCloseDevice,
    CouldNotOpenDLL,
    CouldNotConnectToDevice,
    CouldNotCallFunction,
    CouldNotErase,
    CouldNotProgram,
    CouldNotRead,
    CouldNotOpenHexFile,
    WrongMagicNumber
} errorcodes;

static name_map_t nrfjprog_js_err_map = {
    { errorcodes::JsSuccess, "Success" },
    { errorcodes::CouldNotFindJlinkDLL, "CouldNotFindJlinkDLL" },
    { errorcodes::CouldNotFindJprogDLL, "CouldNotFindJprogDLL" },
    { errorcodes::CouldNotLoadDLL, "CouldNotLoadDLL" },
    { errorcodes::CouldNotOpenDLL, "CouldNotOpenDLL" },
    { errorcodes::CouldNotOpenDevice, "CouldNotOpenDevice" },
    { errorcodes::CouldNotResetDevice, "CouldNotResetDevice" },
    { errorcodes::CouldNotCloseDevice, "CouldNotCloseDevice" },
    { errorcodes::CouldNotConnectToDevice, "CouldNotConnectToDevice" },
    { errorcodes::CouldNotCallFunction, "CouldNotCallFunction" },
    { errorcodes::CouldNotErase, "CouldNotErase" },
    { errorcodes::CouldNotProgram, "CouldNotProgram" },
    { errorcodes::CouldNotRead, "CouldNotRead" },
    { errorcodes::CouldNotOpenHexFile, "CouldNotOpenHexFile" },
    { errorcodes::WrongMagicNumber, "WrongMagicNumber" }
};

#endif
