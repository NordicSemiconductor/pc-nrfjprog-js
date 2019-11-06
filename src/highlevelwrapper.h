/* Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
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

#ifndef HIGHLEVELWRAPPER_H
#define HIGHLEVELWRAPPER_H

#include "highlevel_common.h"
#include "highlevelnrfjprogdll.h"
#include "utility/errormessage.h"

typedef nrfjprogdll_err_t (*HILVL_nRFJ_dll_get_version)(uint32_t *major, uint32_t *minor,
                                                        uint32_t *micro);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_dll_open)(const char *default_jlink_path,
                                                 log_callback *log_cb);
typedef void (*HILVL_nRFJ_dll_close)(void);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_is_dll_open)(bool *is_opened);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_get_connected_probes)(uint32_t serial_numbers[],
                                                             uint32_t serial_numbers_len,
                                                             uint32_t *num_available);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_probe_init)(Probe_handle_t *debug_probe,
                                                   progress_callback * prog_cb,
                                                   log_callback *log_cb, uint32_t snr,
                                                   const char *jlink_path);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_probe_set_coprocessor)(Probe_handle_t debug_probe,
                                                              coprocessor_t coprocessor);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_dfu_init)(Probe_handle_t *dfu_probe,
                                                 progress_callback * prog_cb,
                                                 log_callback *log_cb,
                                                 uint32_t snr, coprocessor_t coprocessor,
                                                 const char *jlink_path);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_mcuboot_dfu_init)(Probe_handle_t *dfu_probe,
                                            progress_callback * prog_cb,
                                            log_callback *log_cb,
                                            const char * uart,
                                            uint32_t baud_rate,
                                            uint32_t response_timeout);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_probe_uninit)(Probe_handle_t *debug_probe);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_get_library_info)(Probe_handle_t debug_probe,
                                                         library_info_t *library_info);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_get_probe_info)(Probe_handle_t debug_probe,
                                                       probe_info_t *probe_info);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_get_device_info)(Probe_handle_t debug_probe,
                                                        device_info_t *device_info);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_program)(Probe_handle_t debug_probe, const char *hex_path,
                                                program_options_t program_options);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_read_to_file)(Probe_handle_t debug_probe,
                                                     const char *hex_path,
                                                     read_options_t read_options);
typedef nrfjprogdll_err_t (*HiLVL_nRFJ_verify)(Probe_handle_t debug_probe, const char *hex_path,
                                               verify_action_t verify_action);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_erase)(Probe_handle_t debug_probe,
                                              erase_action_t erase_action, uint32_t start_address,
                                              uint32_t end_address);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_recover)(Probe_handle_t debug_probe);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_read)(Probe_handle_t debug_probe, uint32_t addr,
                                             uint8_t *data, uint32_t data_len);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_read_u32)(Probe_handle_t debug_probe, uint32_t addr,
                                                 uint32_t *data);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_write)(Probe_handle_t debug_probe, uint32_t addr,
                                              const uint8_t *data, uint32_t data_len);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_write_u32)(Probe_handle_t debug_probe, uint32_t addr,
                                                  const uint32_t data);
typedef nrfjprogdll_err_t (*HILVL_nRFJ_reset)(Probe_handle_t debug_probe,
                                              reset_action_t reset_action);

struct LibraryFunctionPointersType
{
    HILVL_nRFJ_dll_get_version dll_get_version;
    HILVL_nRFJ_dll_open dll_open;
    HILVL_nRFJ_dll_close dll_close;
    HILVL_nRFJ_is_dll_open is_dll_open;
    HILVL_nRFJ_get_connected_probes get_connected_probes;
    HILVL_nRFJ_probe_init probe_init;
    HILVL_nRFJ_probe_set_coprocessor probe_set_coprocessor;
    HILVL_nRFJ_dfu_init dfu_init;
    HILVL_nRFJ_mcuboot_dfu_init mcuboot_dfu_init;
    HILVL_nRFJ_probe_uninit probe_uninit;
    HILVL_nRFJ_get_library_info get_library_info;
    HILVL_nRFJ_get_probe_info get_probe_info;
    HILVL_nRFJ_get_device_info get_device_info;
    HILVL_nRFJ_program program;
    HILVL_nRFJ_read_to_file read_to_file;
    HiLVL_nRFJ_verify verify;
    HILVL_nRFJ_erase erase;
    HILVL_nRFJ_recover recover;
    HILVL_nRFJ_read read;
    HILVL_nRFJ_read_u32 read_u32;
    HILVL_nRFJ_write write;
    HILVL_nRFJ_write_u32 write_u32;
    HILVL_nRFJ_reset reset;
};

errorcode_t loadHighLevelFunctions(LibraryFunctionPointersType *libraryFunctions);
errorcode_t releaseHighLevel();

#endif
