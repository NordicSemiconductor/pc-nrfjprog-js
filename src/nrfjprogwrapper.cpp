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
std::string nrfjprogPath;

#define LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(target, name, handle) do { \
    if (!load_func_ptr((target), (name), (handle))) { \
        return errorcode_t::CouldNotLoadDLL; \
    } \
} while (0);

errorcode_t loadnRFjprogFunctions(nRFjprogLibraryFunctionPointersType * libraryFunctions)
{
    if (nrfjprogPath.empty()) {
        std::string libarayName = getnrfjprogLibraryName();
        const errorcode_t finderror = OSFilesFindLibrary(nrfjprogPath, libarayName);

        if (finderror != errorcode_t::JsSuccess) {
            return finderror;
        }
    }

    nrfJproglibraryHandle = LibraryLoad(nrfjprogPath);

    if (!nrfJproglibraryHandle) {
        return errorcode_t::CouldNotLoadDLL;
    }

    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->dll_version, "NRFJPROG_dll_version", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->is_dll_open, "NRFJPROG_is_dll_open", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->open_dll, "NRFJPROG_open_dll", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->close_dll, "NRFJPROG_close_dll", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->enum_emu_snr, "NRFJPROG_enum_emu_snr", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->is_connected_to_emu, "NRFJPROG_is_connected_to_emu", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->connect_to_emu_with_snr, "NRFJPROG_connect_to_emu_with_snr", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->connect_to_emu_without_snr, "NRFJPROG_connect_to_emu_without_snr", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_connected_emu_snr, "NRFJPROG_read_connected_emu_snr", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_connected_emu_fwstr, "NRFJPROG_read_connected_emu_fwstr", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->disconnect_from_emu, "NRFJPROG_disconnect_from_emu", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->recover, "NRFJPROG_recover", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->is_connected_to_device, "NRFJPROG_is_connected_to_device", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->connect_to_device, "NRFJPROG_connect_to_device", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->disconnect_from_device, "NRFJPROG_disconnect_from_device", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->readback_protect, "NRFJPROG_readback_protect", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->readback_status, "NRFJPROG_readback_status", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_region_0_size_and_source, "NRFJPROG_read_region_0_size_and_source", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->debug_reset, "NRFJPROG_debug_reset", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->sys_reset, "NRFJPROG_sys_reset", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->pin_reset, "NRFJPROG_pin_reset", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->disable_bprot, "NRFJPROG_disable_bprot", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->erase_all, "NRFJPROG_erase_all", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->erase_page, "NRFJPROG_erase_page", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->erase_uicr, "NRFJPROG_erase_uicr", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->write_u32, "NRFJPROG_write_u32", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_u32, "NRFJPROG_read_u32", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->write, "NRFJPROG_write", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read, "NRFJPROG_read", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->is_halted, "NRFJPROG_is_halted", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->halt, "NRFJPROG_halt", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->run, "NRFJPROG_run", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->go, "NRFJPROG_go", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->step, "NRFJPROG_step", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_ram_sections_count, "NRFJPROG_read_ram_sections_count", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_ram_sections_size, "NRFJPROG_read_ram_sections_size", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_ram_sections_power_status, "NRFJPROG_read_ram_sections_power_status", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->is_ram_powered, "NRFJPROG_is_ram_powered", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->power_ram_all, "NRFJPROG_power_ram_all", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->unpower_ram_section, "NRFJPROG_unpower_ram_section", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_cpu_register, "NRFJPROG_read_cpu_register", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->write_cpu_register, "NRFJPROG_write_cpu_register", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_device_version, "NRFJPROG_read_device_version", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_device_family, "NRFJPROG_read_device_family", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_debug_port_register, "NRFJPROG_read_debug_port_register", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->write_debug_port_register, "NRFJPROG_write_debug_port_register", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->read_access_port_register, "NRFJPROG_read_access_port_register", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->write_access_port_register, "NRFJPROG_write_access_port_register", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->is_rtt_started, "NRFJPROG_is_rtt_started", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->rtt_set_control_block_address, "NRFJPROG_rtt_set_control_block_address", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->rtt_start, "NRFJPROG_rtt_start", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->rtt_is_control_block_found, "NRFJPROG_rtt_is_control_block_found", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->rtt_stop, "NRFJPROG_rtt_stop", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->rtt_read, "NRFJPROG_rtt_read", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->rtt_write, "NRFJPROG_rtt_write", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->rtt_read_channel_count, "NRFJPROG_rtt_read_channel_count", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->rtt_read_channel_info, "NRFJPROG_rtt_read_channel_info", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->is_qspi_init, "NRFJPROG_is_qspi_init", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->qspi_init, "NRFJPROG_qspi_init", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->qspi_uninit, "NRFJPROG_qspi_uninit", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->qspi_read, "NRFJPROG_qspi_read", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->qspi_write, "NRFJPROG_qspi_write", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->qspi_erase, "NRFJPROG_qspi_erase", nrfJproglibraryHandle);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(&libraryFunctions->qspi_custom, "NRFJPROG_qspi_custom", nrfJproglibraryHandle);

    return errorcode_t::JsSuccess;
}

errorcode_t releasenRFjprog() {
    LibraryFree(nrfJproglibraryHandle);
    return errorcode_t::JsSuccess;
}
