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

errorcode_t loadnRFjprogFunctions(nRFjprogDllFunctionPointersType * dll_function)
{
    if (nrfjprogPath.empty()) {
        std::string libarayName = getnrfjprogLibraryName();
        const errorcode_t finderror = OSFilesFindDll(nrfjprogPath, libarayName);

        if (finderror != errorcode_t::JsSuccess) {
            return finderror;
        }
    }

    nrfJproglibraryHandle = LibraryLoad(nrfjprogPath);

    if (!nrfJproglibraryHandle){
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->dll_version, "NRFJPROG_dll_version", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->is_dll_open, "NRFJPROG_is_dll_open", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->open_dll, "NRFJPROG_open_dll", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->close_dll, "NRFJPROG_close_dll", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->enum_emu_snr, "NRFJPROG_enum_emu_snr", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->is_connected_to_emu, "NRFJPROG_is_connected_to_emu", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->connect_to_emu_with_snr, "NRFJPROG_connect_to_emu_with_snr", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->connect_to_emu_without_snr, "NRFJPROG_connect_to_emu_without_snr", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_connected_emu_snr, "NRFJPROG_read_connected_emu_snr", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_connected_emu_fwstr, "NRFJPROG_read_connected_emu_fwstr", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->disconnect_from_emu, "NRFJPROG_disconnect_from_emu", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->recover, "NRFJPROG_recover", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->is_connected_to_device, "NRFJPROG_is_connected_to_device", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->connect_to_device, "NRFJPROG_connect_to_device", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->disconnect_from_device, "NRFJPROG_disconnect_from_device", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->readback_protect, "NRFJPROG_readback_protect", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->readback_status, "NRFJPROG_readback_status", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_region_0_size_and_source, "NRFJPROG_read_region_0_size_and_source", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->debug_reset, "NRFJPROG_debug_reset", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->sys_reset, "NRFJPROG_sys_reset", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->pin_reset, "NRFJPROG_pin_reset", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->disable_bprot, "NRFJPROG_disable_bprot", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->erase_all, "NRFJPROG_erase_all", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->erase_page, "NRFJPROG_erase_page", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->erase_uicr, "NRFJPROG_erase_uicr", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->write_u32, "NRFJPROG_write_u32", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_u32, "NRFJPROG_read_u32", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->write, "NRFJPROG_write", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read, "NRFJPROG_read", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->is_halted, "NRFJPROG_is_halted", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->halt, "NRFJPROG_halt", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->run, "NRFJPROG_run", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->go, "NRFJPROG_go", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->step, "NRFJPROG_step", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_ram_sections_count, "NRFJPROG_read_ram_sections_count", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_ram_sections_size, "NRFJPROG_read_ram_sections_size", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_ram_sections_power_status, "NRFJPROG_read_ram_sections_power_status", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->is_ram_powered, "NRFJPROG_is_ram_powered", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->power_ram_all, "NRFJPROG_power_ram_all", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->unpower_ram_section, "NRFJPROG_unpower_ram_section", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_cpu_register, "NRFJPROG_read_cpu_register", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->write_cpu_register, "NRFJPROG_write_cpu_register", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_device_version, "NRFJPROG_read_device_version", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_device_family, "NRFJPROG_read_device_family", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_debug_port_register, "NRFJPROG_read_debug_port_register", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->write_debug_port_register, "NRFJPROG_write_debug_port_register", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_access_port_register, "NRFJPROG_read_access_port_register", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->write_access_port_register, "NRFJPROG_write_access_port_register", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->is_rtt_started, "NRFJPROG_is_rtt_started", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->rtt_set_control_block_address, "NRFJPROG_rtt_set_control_block_address", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->rtt_start, "NRFJPROG_rtt_start", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->rtt_is_control_block_found, "NRFJPROG_rtt_is_control_block_found", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->rtt_stop, "NRFJPROG_rtt_stop", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->rtt_read, "NRFJPROG_rtt_read", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->rtt_write, "NRFJPROG_rtt_write", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->rtt_read_channel_count, "NRFJPROG_rtt_read_channel_count", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->rtt_read_channel_info, "NRFJPROG_rtt_read_channel_info", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->is_qspi_init, "NRFJPROG_is_qspi_init", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->qspi_init, "NRFJPROG_qspi_init", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->qspi_uninit, "NRFJPROG_qspi_uninit", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->qspi_read, "NRFJPROG_qspi_read", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->qspi_write, "NRFJPROG_qspi_write", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->qspi_erase, "NRFJPROG_qspi_erase", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->qspi_custom, "NRFJPROG_qspi_custom", nrfJproglibraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    return errorcode_t::JsSuccess;
}

errorcode_t releasenRFjprog() {
    LibraryFree(nrfJproglibraryHandle);
    return errorcode_t::JsSuccess;
}
