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

#ifndef NRFJPROGWRAPPER_H
#define NRFJPROGWRAPPER_H

#include "highlevel_common.h"
#include "utility/errormessage.h"

typedef nrfjprogdll_err_t (*nRFjprog_open_dll)(const char *jlink_path, msg_callback *cb,
                                               device_family_t family);
typedef void (*nRFjprog_close_dll)(void);
typedef nrfjprogdll_err_t (*nRFjprog_connect_to_emu_with_snr)(uint32_t serial_number,
                                                              uint32_t clock_speed_in_khz);
typedef nrfjprogdll_err_t (*nRFjprog_disconnect_from_emu)(void);
typedef nrfjprogdll_err_t (*nRFjprog_is_connected_to_device)(bool *is_emu_connected_to_device);
typedef nrfjprogdll_err_t (*nRFjprog_connect_to_device)(void);
typedef nrfjprogdll_err_t (*nRFjprog_is_rtt_started)(bool *started);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_set_control_block_address)(uint32_t address);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_start)(void);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_is_control_block_found)(bool *is_control_block_found);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_stop)(void);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_read)(uint32_t up_channel_index, char *data,
                                               uint32_t data_len, uint32_t *data_read);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_write)(uint32_t down_channel_index, const char *data,
                                                uint32_t data_len, uint32_t *data_written);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_read_channel_count)(uint32_t *down_channel_number,
                                                             uint32_t *up_channel_number);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_read_channel_info)(uint32_t channel_index,
                                                            rtt_direction_t dir, char *channel_name,
                                                            uint32_t channel_name_len,
                                                            uint32_t *channel_size);

struct nRFjprogLibraryFunctionPointersType
{
    nRFjprog_open_dll open_dll;
    nRFjprog_close_dll close_dll;
    nRFjprog_connect_to_emu_with_snr connect_to_emu_with_snr;
    nRFjprog_disconnect_from_emu disconnect_from_emu;
    nRFjprog_is_connected_to_device is_connected_to_device;
    nRFjprog_connect_to_device connect_to_device;
    nRFjprog_is_rtt_started is_rtt_started;
    nRFjprog_rtt_set_control_block_address rtt_set_control_block_address;
    nRFjprog_rtt_start rtt_start;
    nRFjprog_rtt_is_control_block_found rtt_is_control_block_found;
    nRFjprog_rtt_stop rtt_stop;
    nRFjprog_rtt_read rtt_read;
    nRFjprog_rtt_write rtt_write;
    nRFjprog_rtt_read_channel_count rtt_read_channel_count;
    nRFjprog_rtt_read_channel_info rtt_read_channel_info;
};

errorcode_t loadnRFjprogFunctions(nRFjprogLibraryFunctionPointersType *libraryFunctions);
errorcode_t releasenRFjprog();

#endif
