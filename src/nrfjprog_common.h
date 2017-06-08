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

#ifndef __NRFJPROG_COMMON_H__
#define __NRFJPROG_COMMON_H__

#include "common.h"
#include "DllCommonDefinitions.h"
#include "nrfjprog.h"

static name_map_t device_version_map = {
    NAME_MAP_ENTRY(UNKNOWN),
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
    NAME_MAP_ENTRY(NRF52_FP2_ENGA)
};

static name_map_t device_family_map = {
    NAME_MAP_ENTRY(NRF51_FAMILY),
    NAME_MAP_ENTRY(NRF52_FAMILY),
    NAME_MAP_ENTRY(UNKNOWN_FAMILY)
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
    NAME_MAP_ENTRY(FAMILY_UNKNOWN),
    NAME_MAP_ENTRY(NVMC_ERROR),
    NAME_MAP_ENTRY(RECOVER_FAILED),
    NAME_MAP_ENTRY(RAM_IS_OFF_ERROR),
    NAME_MAP_ENTRY(QspiIniNotFoundError),
    NAME_MAP_ENTRY(QspiIniCannotBeOpenedError),
    NAME_MAP_ENTRY(QspiSyntaxError),
    NAME_MAP_ENTRY(QspiIniParsingError),
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

#endif // __NRFJPROG_COMMON_H__
