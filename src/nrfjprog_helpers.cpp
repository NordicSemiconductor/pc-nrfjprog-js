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

#include "nrfjprog_helpers.h"

#include "utility/utility.h"
#include "utility/conversion.h"

v8::Local<v8::Object> ProbeInfo::ToJs()
{
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Object> obj = Nan::New<v8::Object>();

    Utility::Set(obj, "serialNumber", Convert::toJsNumber(serial_number));
    Utility::Set(obj, "family", Convert::toJsNumber(family));

    return scope.Escape(obj);
}

EraseOptions::EraseOptions(v8::Local<v8::Object> obj) :
    eraseMode(ERASE_ALL),
    startAddress(0),
    endAddress(0)
{
    if (Utility::Has(obj, "erase_mode"))
    {
        eraseMode = (erase_mode_t)Convert::getNativeUint32(obj, "erase_mode");
    }

    if (Utility::Has(obj, "start_adress"))
    {
        startAddress = (erase_mode_t)Convert::getNativeUint32(obj, "erase_mode");
    }

    if (Utility::Has(obj, "end_address"))
    {
        endAddress = (erase_mode_t)Convert::getNativeUint32(obj, "end_address");
    }
}

ReadToFileOptions::ReadToFileOptions(v8::Local<v8::Object> obj)
{
    options.readram = false;
    options.readcode = false;
    options.readuicr = false;
    options.readqspi = false;

    if (Utility::Has(obj, "readram"))
    {
        options.readram = Convert::getBool(obj, "readram");
    }

    if (Utility::Has(obj, "readcode"))
    {
        options.readcode = Convert::getBool(obj, "readcode");
    }

    if (Utility::Has(obj, "readuicr"))
    {
        options.readuicr = Convert::getBool(obj, "readuicr");
    }

    if (Utility::Has(obj, "readqspi"))
    {
        options.readqspi = Convert::getBool(obj, "readqspi");
    }
}

ProgramOptions::ProgramOptions(v8::Local<v8::Object> obj)
{
    options.verify = true;
    options.chip_erase_mode = ERASE_ALL;
    options.qspi_erase_mode = ERASE_NONE;
    options.reset = true;

    if (Utility::Has(obj, "verify"))
    {
        options.verify = Convert::getBool(obj, "verify");
    }

    if (Utility::Has(obj, "chip_erase_mode"))
    {
        options.chip_erase_mode = (erase_mode_t)Convert::getNativeUint32(obj, "chip_erase_mode");
    }

    if (Utility::Has(obj, "qspi_erase_mode"))
    {
        options.qspi_erase_mode = (erase_mode_t)Convert::getNativeUint32(obj, "qspi_erase_mode");
    }

    if (Utility::Has(obj, "reset"))
    {
        options.reset = Convert::getBool(obj, "reset");
    }
}
