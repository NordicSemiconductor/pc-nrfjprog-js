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

#ifndef HIGHLEVEL_COMMON_H
#define HIGHLEVEL_COMMON_H

#include "DllCommonDefinitions.h"
#include "common.h"

using input_format_t = enum { INPUT_FORMAT_HEX_FILE, INPUT_FORMAT_HEX_STRING };

using errorcode_t = enum {
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
    CouldNotExecuteDueToLoad
};

using RTTErrorcodes_t = enum RTTErrorcodes {
    RTTSuccess,
    RTTCouldNotLoadHighlevelLibrary,
    RTTCouldNotOpenHighlevelLibrary,
    RTTCouldNotGetDeviceInformation,
    RTTCouldNotLoadnRFjprogLibrary,
    RTTCouldNotOpennRFjprogLibrary,
    RTTCouldNotConnectToDevice,
    RTTCouldNotStartRTT,
    RTTCouldNotFindControlBlock,
    RTTCouldNotGetChannelInformation,
    RTTCouldNotCallFunction,
    RTTNotInitialized,
    RTTCouldNotExecuteDueToLoad
};

static name_map_t nrfjprog_js_err_map = {
    {errorcode_t::JsSuccess, "Success"},
    {errorcode_t::CouldNotFindJlinkDLL, "CouldNotFindJlinkDLL"},
    {errorcode_t::CouldNotFindJprogDLL, "CouldNotFindJprogDLL"},
    {errorcode_t::CouldNotLoadDLL, "CouldNotLoadDLL"},
    {errorcode_t::CouldNotOpenDLL, "CouldNotOpenDLL"},
    {errorcode_t::CouldNotOpenDevice, "CouldNotOpenDevice"},
    {errorcode_t::CouldNotResetDevice, "CouldNotResetDevice"},
    {errorcode_t::CouldNotCloseDevice, "CouldNotCloseDevice"},
    {errorcode_t::CouldNotConnectToDevice, "CouldNotConnectToDevice"},
    {errorcode_t::CouldNotCallFunction, "CouldNotCallFunction"},
    {errorcode_t::CouldNotErase, "CouldNotErase"},
    {errorcode_t::CouldNotProgram, "CouldNotProgram"},
    {errorcode_t::CouldNotRead, "CouldNotRead"},
    {errorcode_t::CouldNotOpenHexFile, "CouldNotOpenHexFile"},
    {errorcode_t::CouldNotExecuteDueToLoad,
     "Could not execute the function due to too many calls in line"}};

static name_map_t rtt_err_map = {
    {RTTSuccess, "Success"},
    {RTTCouldNotLoadHighlevelLibrary, "Could Not Load Highlevel Library"},
    {RTTCouldNotOpenHighlevelLibrary, "Could Not Open Highlevel Library"},
    {RTTCouldNotGetDeviceInformation, "Could Not Get Device Information"},
    {RTTCouldNotLoadnRFjprogLibrary, "Could Not Load nRFjprog Library"},
    {RTTCouldNotOpennRFjprogLibrary, "Could Not Open nRFjprog Library"},
    {RTTCouldNotConnectToDevice, "Could Not Connect To Device"},
    {RTTCouldNotStartRTT, "Could Not Start RTT"},
    {RTTCouldNotFindControlBlock, "Could Not Find Control Block"},
    {RTTCouldNotGetChannelInformation, "Could Not Get Channel Information"},
    {RTTCouldNotCallFunction, "Could Not Call Function"},
    {RTTNotInitialized, "There is no RTT connection open"},
    {RTTCouldNotExecuteDueToLoad, "Could not execute the function due to too many calls in line"}};

static name_map_t program_parameter_type_map = {NAME_MAP_ENTRY(INPUT_FORMAT_HEX_FILE),
                                                NAME_MAP_ENTRY(INPUT_FORMAT_HEX_STRING)};

static name_map_t device_version_map = {NAME_MAP_ENTRY(UNKNOWN),
                                        NAME_MAP_ENTRY(NRF51xxx_xxAA_REV1),
                                        NAME_MAP_ENTRY(NRF51xxx_xxAA_REV2),
                                        NAME_MAP_ENTRY(NRF51xxx_xxAA_REV3),
                                        NAME_MAP_ENTRY(NRF51xxx_xxAB_REV3),
                                        NAME_MAP_ENTRY(NRF51xxx_xxAC_REV3),
                                        NAME_MAP_ENTRY(NRF51802_xxAA_REV3),
                                        NAME_MAP_ENTRY(NRF51801_xxAB_REV3),
                                        NAME_MAP_ENTRY(NRF51_XLR1),
                                        NAME_MAP_ENTRY(NRF51_XLR2),
                                        NAME_MAP_ENTRY(NRF51_XLR3),
                                        NAME_MAP_ENTRY(NRF51_L3),
                                        NAME_MAP_ENTRY(NRF51_XLR3P),
                                        NAME_MAP_ENTRY(NRF51_XLR3LC),
                                        NAME_MAP_ENTRY(NRF52832_xxAA_ENGA),
                                        NAME_MAP_ENTRY(NRF52832_xxAA_ENGB),
                                        NAME_MAP_ENTRY(NRF52832_xxAA_REV1),
                                        NAME_MAP_ENTRY(NRF52832_xxAB_REV1),
                                        NAME_MAP_ENTRY(NRF52832_xxAA_FUTURE),
                                        NAME_MAP_ENTRY(NRF52832_xxAB_FUTURE),
                                        NAME_MAP_ENTRY(NRF52840_xxAA_ENGA),
                                        NAME_MAP_ENTRY(NRF52840_xxAA_FUTURE),
                                        NAME_MAP_ENTRY(NRF52810_xxAA_REV1),
                                        NAME_MAP_ENTRY(NRF52810_xxAA_FUTURE),
                                        NAME_MAP_ENTRY(NRF52_FP1_ENGA),
                                        NAME_MAP_ENTRY(NRF52_FP1_ENGB),
                                        NAME_MAP_ENTRY(NRF52_FP1),
                                        NAME_MAP_ENTRY(NRF52_FP1_FUTURE),
                                        NAME_MAP_ENTRY(NRF52_FP2_ENGA)};

static name_map_t device_family_map = {NAME_MAP_ENTRY(NRF51_FAMILY), NAME_MAP_ENTRY(NRF52_FAMILY),
                                       NAME_MAP_ENTRY(UNKNOWN_FAMILY)};

static name_map_t nrfjprogdll_err_map = {NAME_MAP_ENTRY(SUCCESS),
                                         NAME_MAP_ENTRY(OUT_OF_MEMORY),
                                         NAME_MAP_ENTRY(INVALID_OPERATION),
                                         NAME_MAP_ENTRY(INVALID_PARAMETER),
                                         NAME_MAP_ENTRY(INVALID_DEVICE_FOR_OPERATION),
                                         NAME_MAP_ENTRY(WRONG_FAMILY_FOR_DEVICE),
                                         NAME_MAP_ENTRY(EMULATOR_NOT_CONNECTED),
                                         NAME_MAP_ENTRY(CANNOT_CONNECT),
                                         NAME_MAP_ENTRY(LOW_VOLTAGE),
                                         NAME_MAP_ENTRY(NO_EMULATOR_CONNECTED),
                                         NAME_MAP_ENTRY(NVMC_ERROR),
                                         NAME_MAP_ENTRY(RECOVER_FAILED),
                                         NAME_MAP_ENTRY(RAM_IS_OFF_ERROR),
                                         NAME_MAP_ENTRY(NOT_AVAILABLE_BECAUSE_PROTECTION),
                                         NAME_MAP_ENTRY(NOT_AVAILABLE_BECAUSE_MPU_CONFIG),
                                         NAME_MAP_ENTRY(JLINKARM_DLL_NOT_FOUND),
                                         NAME_MAP_ENTRY(JLINKARM_DLL_COULD_NOT_BE_OPENED),
                                         NAME_MAP_ENTRY(JLINKARM_DLL_ERROR),
                                         NAME_MAP_ENTRY(JLINKARM_DLL_TOO_OLD),
                                         NAME_MAP_ENTRY(NRFJPROG_SUB_DLL_NOT_FOUND),
                                         NAME_MAP_ENTRY(NRFJPROG_SUB_DLL_COULD_NOT_BE_OPENED),
                                         NAME_MAP_ENTRY(NRFJPROG_SUB_DLL_COULD_NOT_LOAD_FUNCTIONS),
                                         NAME_MAP_ENTRY(VERIFY_ERROR),
                                         NAME_MAP_ENTRY(NOT_IMPLEMENTED_ERROR)};

#endif // __NRFJPROG_COMMON_H__
