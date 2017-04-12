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

#include <chrono>

#include "DllCommonDefinitions.h"
#include "nrfjprog.h"

#include "common.h"

static name_map_t cpu_registers_map = {
    NAME_MAP_ENTRY(R0),
	NAME_MAP_ENTRY(R1),
	NAME_MAP_ENTRY(R2),
	NAME_MAP_ENTRY(R3),
	NAME_MAP_ENTRY(R4),
	NAME_MAP_ENTRY(R5),
	NAME_MAP_ENTRY(R6),
	NAME_MAP_ENTRY(R7),
	NAME_MAP_ENTRY(R8),
	NAME_MAP_ENTRY(R9),
	NAME_MAP_ENTRY(R10),
	NAME_MAP_ENTRY(R11),
	NAME_MAP_ENTRY(R12),
	NAME_MAP_ENTRY(R13),
	NAME_MAP_ENTRY(R14),
	NAME_MAP_ENTRY(R15),
	NAME_MAP_ENTRY(XPSR),
	NAME_MAP_ENTRY(MSP),
	NAME_MAP_ENTRY(PSP)
};

static name_map_t ram_section_power_status_map = {
    NAME_MAP_ENTRY(RAM_OFF),
    NAME_MAP_ENTRY(RAM_ON)
};

static name_map_t readback_protection_status_map = {
    NAME_MAP_ENTRY(NONE),
	NAME_MAP_ENTRY(REGION_0),
	NAME_MAP_ENTRY(ALL),
	NAME_MAP_ENTRY(BOTH)
};

static name_map_t region_0_source_map = {
    NAME_MAP_ENTRY(NO_REGION_0),
    NAME_MAP_ENTRY(FACTORY),
    NAME_MAP_ENTRY(USER)
};

static name_map_t device_version_map = {
    NAME_MAP_ENTRY(UNKNOWN),
    NAME_MAP_ENTRY(NRF51_XLR1),
    NAME_MAP_ENTRY(NRF51_XLR2),
    NAME_MAP_ENTRY(NRF51_XLR3),
    NAME_MAP_ENTRY(NRF51_L3),
    NAME_MAP_ENTRY(NRF51_XLR3P),
    NAME_MAP_ENTRY(NRF51_XLR3LC),
    NAME_MAP_ENTRY(NRF52_FP1_ENGA),
    NAME_MAP_ENTRY(NRF52_FP1_ENGB),
    NAME_MAP_ENTRY(NRF52_FP1)
};

static name_map_t device_family_map = {
    NAME_MAP_ENTRY(NRF51_FAMILY),
    NAME_MAP_ENTRY(NRF52_FAMILY),
    NAME_MAP_ENTRY(UNKNOWN_FAMILY)
};

static name_map_t rtt_direction_map = {
    NAME_MAP_ENTRY(UP_DIRECTION),
    NAME_MAP_ENTRY(DOWN_DIRECTION)
};

static name_map_t nrfjprogdll_err_map = {
    NAME_MAP_ENTRY(SUCCESS),
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
    NAME_MAP_ENTRY(NOT_AVAILABLE_BECAUSE_PROTECTION),
    NAME_MAP_ENTRY(JLINKARM_DLL_NOT_FOUND),
    NAME_MAP_ENTRY(JLINKARM_DLL_COULD_NOT_BE_OPENED),
    NAME_MAP_ENTRY(JLINKARM_DLL_ERROR),
    NAME_MAP_ENTRY(JLINKARM_DLL_TOO_OLD),
    NAME_MAP_ENTRY(NRFJPROG_SUB_DLL_NOT_FOUND),
    NAME_MAP_ENTRY(NRFJPROG_SUB_DLL_COULD_NOT_BE_OPENED),
    NAME_MAP_ENTRY(NOT_IMPLEMENTED_ERROR)
};

static name_map_t NrfjprogErrorCodesTypeMap = {
    NAME_MAP_ENTRY(Success),
    NAME_MAP_ENTRY(NrfjprogError),
    NAME_MAP_ENTRY(NrfjprogOutdatedError),
    NAME_MAP_ENTRY(MemoryAllocationError),
    NAME_MAP_ENTRY(InvalidArgumentError),
    NAME_MAP_ENTRY(InsufficientArgumentsError),
    NAME_MAP_ENTRY(IncompatibleArgumentsError),
    NAME_MAP_ENTRY(DuplicatedArgumentsError),
    NAME_MAP_ENTRY(NoOperationError),
    NAME_MAP_ENTRY(UnavailableOperationBecauseProtectionError),
    NAME_MAP_ENTRY(UnavailableOperationInFamilyError),
    NAME_MAP_ENTRY(WrongFamilyForDeviceError),
    NAME_MAP_ENTRY(NrfjprogDllNotFoundError),
    NAME_MAP_ENTRY(NrfjprogDllLoadFailedError),
    NAME_MAP_ENTRY(NrfjprogDllFunctionLoadFailedError),
    NAME_MAP_ENTRY(NrfjprogDllNotImplementedError),
    NAME_MAP_ENTRY(NrfjprogIniNotFoundError),
    NAME_MAP_ENTRY(JLinkARMDllNotFoundError),
    NAME_MAP_ENTRY(JLinkARMDllInvalidError),
    NAME_MAP_ENTRY(JLinkARMDllFailedToOpenError),
    NAME_MAP_ENTRY(JLinkARMDllError),
    NAME_MAP_ENTRY(JLinkARMDllTooOldError),
    NAME_MAP_ENTRY(InvalidSerialNumberError),
    NAME_MAP_ENTRY(NoDebuggersError),
    NAME_MAP_ENTRY(NotPossibleToConnectError),
    NAME_MAP_ENTRY(LowVoltageError),
    NAME_MAP_ENTRY(FileNotFoundError),
    NAME_MAP_ENTRY(InvalidHexFileError),
    NAME_MAP_ENTRY(FicrReadError),
    NAME_MAP_ENTRY(WrongArgumentError),
    NAME_MAP_ENTRY(VerifyError),
    NAME_MAP_ENTRY(NoWritePermissionError),
    NAME_MAP_ENTRY(NVMCOperationError),
    NAME_MAP_ENTRY(FlashNotErasedError),
    NAME_MAP_ENTRY(RamIsOffError),
    NAME_MAP_ENTRY(FicrOperationWarning),
    NAME_MAP_ENTRY(UnalignedPageEraseWarning),
    NAME_MAP_ENTRY(NoLogWarning),
    NAME_MAP_ENTRY(UicrWriteOperationWithoutEraseWarning)
};

const std::string getCurrentTimeInMilliseconds()
{
    auto current_time = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(current_time);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch());

    auto ttm = gmtime(&time);

    char date_time_format[] = "%Y-%m-%dT%H:%M:%S";
    char time_str[20] = "";

    strftime(time_str, 20, date_time_format, ttm);

    std::string result(time_str);
    result.append(".");

    char millisecond_str[4];
    sprintf(millisecond_str, "%03d", static_cast<int>(ms.count() % 1000));
    result.append(millisecond_str);
    result.append("Z");

    return result;
}

uint16_t uint16_decode(const uint8_t *p_encoded_data)
{
        return ( (static_cast<uint16_t>(const_cast<uint8_t *>(p_encoded_data)[0])) |
                 (static_cast<uint16_t>(const_cast<uint8_t *>(p_encoded_data)[1]) << 8 ));
}

uint32_t uint32_decode(const uint8_t *p_encoded_data)
{
    return ((static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[0]) << 0)  |
            (static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[1]) << 8)  |
            (static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[2]) << 16) |
            (static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[3]) << 24));
}

uint16_t fromNameToValue(name_map_t names, const char *name)
{
    name_map_it_t it;
    uint16_t key = -1;

    for (it = names.begin(); it != names.end(); ++it)
    {
        if (strcmp(it->second, name) == 0)
        {
            key = it->first;
            break;
        }
    }

    return key;
}
