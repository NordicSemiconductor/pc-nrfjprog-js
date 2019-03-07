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

#include "nrfjprogwrapper.h"
#include "libraryloader.h"
#include "osfiles.h"

LibraryHandleType nrfJproglibraryHandle;

errorcode_t loadnRFjprogFunctions(nRFjprogLibraryFunctionPointersType *libraryFunctions)
{
    static std::string nrfjprogPath{};

    if (nrfjprogPath.empty())
    {
        std::string libraryName     = getnrfjprogLibraryName();
        const errorcode_t finderror = OSFilesFindLibrary(nrfjprogPath, libraryName);

        if (finderror != errorcode_t::JsSuccess)
        {
            return finderror;
        }
    }

    nrfJproglibraryHandle = LibraryLoad(nrfjprogPath);

    if (!static_cast<bool>(nrfJproglibraryHandle))
    {
        return errorcode_t::CouldNotLoadDLL;
    }

#define LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(target)                                              \
    do                                                                                             \
    {                                                                                              \
        if (!load_func_ptr(&libraryFunctions->target,                                              \
                           static_cast<const char *>("NRFJPROG_" #target),                         \
                           (nrfJproglibraryHandle)))                                               \
        {                                                                                          \
            return errorcode_t::CouldNotLoadDLL;                                                   \
        }                                                                                          \
    } while (0);

    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(dll_version);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(is_dll_open);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(open_dll);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(close_dll);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(enum_emu_snr);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(is_connected_to_emu);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(connect_to_emu_with_snr);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(connect_to_emu_without_snr);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_connected_emu_snr);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_connected_emu_fwstr);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(disconnect_from_emu);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(recover);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(is_connected_to_device);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(connect_to_device);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(disconnect_from_device);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(readback_protect);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(readback_status);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_region_0_size_and_source);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(debug_reset);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(sys_reset);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(pin_reset);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(disable_bprot);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(erase_all);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(erase_page);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(erase_uicr);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(write_u32);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_u32);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(write);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(is_halted);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(halt);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(run);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(go);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(step);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_ram_sections_count);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_ram_sections_size);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_ram_sections_power_status);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(is_ram_powered);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(power_ram_all);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(unpower_ram_section);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_cpu_register);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(write_cpu_register);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_device_version);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_device_family);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_debug_port_register);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(write_debug_port_register);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_access_port_register);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(write_access_port_register);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(is_rtt_started);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(rtt_set_control_block_address);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(rtt_start);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(rtt_is_control_block_found);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(rtt_stop);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(rtt_read);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(rtt_write);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(rtt_read_channel_count);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(rtt_read_channel_info);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(is_qspi_init);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(qspi_init);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(qspi_uninit);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(qspi_read);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(qspi_write);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(qspi_erase);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(qspi_custom);

#undef LOAD_FUNCTION_POINTER_RETURN_ON_ERROR

    return errorcode_t::JsSuccess;
}

errorcode_t releasenRFjprog()
{
    LibraryFree(nrfJproglibraryHandle);
    return errorcode_t::JsSuccess;
}
