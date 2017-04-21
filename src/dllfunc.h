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

#ifndef DLL_FUNC_PTRS_H
#define DLL_FUNC_PTRS_H

#include "hilvlnrfjprogdll.h"

typedef nrfjprogdll_err_t (*HILVL_nRFJ_dll_get_version)     (uint32_t * major, uint32_t * minor, uint32_t * revision);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_dll_open)            (log_callback * cb);
typedef void              (*HILVL_nRFJ_dll_close)           (void);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_is_dll_open)         (bool * is_opened);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_get_connected_probes)(uint32_t serial_numbers[], uint32_t serial_numbers_len, uint32_t * num_available, const char * jlink_path);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_probe_init)          (Probe_handle_t * debug_probe, uint32_t snr, const char * qspi_ini_path, const char * jlink_path);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_probe_uninit)        (Probe_handle_t * debug_probe);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_probe_get_snr)       (Probe_handle_t debug_probe, uint32_t * snr);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_get_device_family)   (Probe_handle_t debug_probe, device_family_t * family);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_get_device_version)  (Probe_handle_t debug_probe, device_version_t * device);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_program)             (Probe_handle_t debug_probe, const char * hex_path, program_options_t * program_options, progress_callback * fp);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_read_to_file)        (Probe_handle_t debug_probe, const char * hex_path, read_options_t * read_options, progress_callback * fp);
typedef nrfjprogdll_err_t (*HiLVL_nRFJ_verify)              (Probe_handle_t debug_probe, const char * hex_path, progress_callback * fp);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_erase)               (Probe_handle_t debug_probe, erase_mode_t erase_mode, uint32_t start_adress, uint32_t end_adress, progress_callback * fp);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_recover)             (Probe_handle_t debug_probe, progress_callback * fp);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_read)                (Probe_handle_t debug_probe, uint32_t addr, uint8_t * data, uint32_t data_len, progress_callback * fp);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_read_u32)            (Probe_handle_t debug_probe, uint32_t addr, uint32_t * data, progress_callback * fp);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_write)               (Probe_handle_t debug_probe, uint32_t addr, const uint8_t * data, uint32_t data_len, progress_callback * fp);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_write_u32)           (Probe_handle_t debug_probe, uint32_t addr, const uint32_t data, progress_callback * fp);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_reset)               (Probe_handle_t debug_probe);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_run)                 (Probe_handle_t debug_probe);

struct DllFunctionPointersType {
    HILVL_nRFJ_dll_get_version      dll_get_version;
    HILVL_nRFJ_dll_open             dll_open;
    HILVL_nRFJ_dll_close            dll_close;
    HILVL_nRFJ_is_dll_open          is_dll_open;
    HILVL_nRFJ_get_connected_probes get_connected_probes;
    HILVL_nRFJ_probe_init           probe_init;
    HILVL_nRFJ_probe_uninit         probe_uninit;
    HILVL_nRFJ_probe_get_snr        probe_get_snr;
    HILVL_nRFJ_get_device_family    get_device_family;
    HILVL_nRFJ_get_device_version   get_device_version;
    HILVL_nRFJ_program              program;
    HILVL_nRFJ_read_to_file         read_to_file;
    HiLVL_nRFJ_verify               verify;
    HILVL_nRFJ_erase                erase;
    HILVL_nRFJ_recover              recover;
    HILVL_nRFJ_read                 read;
    HILVL_nRFJ_read_u32             read_u32;
    HILVL_nRFJ_write                write;
    HILVL_nRFJ_write_u32            write_u32;
    HILVL_nRFJ_reset                reset;
    HILVL_nRFJ_run                  run;
};


NrfjprogErrorCodesType DllLoad(const char * path, DllFunctionPointersType * dll_function);

void DllFree(void);


#endif
