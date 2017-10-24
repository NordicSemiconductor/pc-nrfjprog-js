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

#ifndef __NRFJPROGWRAPPER_H__
#define __NRFJPROGWRAPPER_H__

//#include "nrfjprogdll.h"
#include "highlevel_common.h"
#include "utility/errormessage.h"

typedef nrfjprogdll_err_t (*nRFjprog_dll_version)                       (uint32_t * major, uint32_t * minor, char * revision);
typedef nrfjprogdll_err_t (*nRFjprog_is_dll_open)                       (bool * opened);
typedef nrfjprogdll_err_t (*nRFjprog_open_dll)                          (const char * jlink_path, msg_callback * cb, device_family_t family);
typedef void              (*nRFjprog_close_dll)                         (void);
typedef nrfjprogdll_err_t (*nRFjprog_enum_emu_snr)                      (uint32_t serial_numbers[], uint32_t serial_numbers_len, uint32_t * num_available);
typedef nrfjprogdll_err_t (*nRFjprog_is_connected_to_emu)               (bool * is_pc_connected_to_emu);
typedef nrfjprogdll_err_t (*nRFjprog_connect_to_emu_with_snr)           (uint32_t serial_number, uint32_t clock_speed_in_khz);
typedef nrfjprogdll_err_t (*nRFjprog_connect_to_emu_without_snr)        (uint32_t clock_speed_in_khz);
typedef nrfjprogdll_err_t (*nRFjprog_read_connected_emu_snr)            (uint32_t * serial_number);
typedef nrfjprogdll_err_t (*nRFjprog_read_connected_emu_fwstr)          (char * buffer, uint32_t buffer_size);
typedef nrfjprogdll_err_t (*nRFjprog_disconnect_from_emu)               (void);
typedef nrfjprogdll_err_t (*nRFjprog_recover)                           (void);
typedef nrfjprogdll_err_t (*nRFjprog_is_connected_to_device)            (bool * is_emu_connected_to_device);
typedef nrfjprogdll_err_t (*nRFjprog_connect_to_device)                 (void);
typedef nrfjprogdll_err_t (*nRFjprog_disconnect_from_device)            (void);
typedef nrfjprogdll_err_t (*nRFjprog_readback_protect)                  (readback_protection_status_t desired_protection);
typedef nrfjprogdll_err_t (*nRFjprog_readback_status)                   (readback_protection_status_t * status);
typedef nrfjprogdll_err_t (*nRFjprog_read_region_0_size_and_source)     (uint32_t * size, region_0_source_t * source);
typedef nrfjprogdll_err_t (*nRFjprog_debug_reset)                       (void);
typedef nrfjprogdll_err_t (*nRFjprog_sys_reset)                         (void);
typedef nrfjprogdll_err_t (*nRFjprog_pin_reset)                         (void);
typedef nrfjprogdll_err_t (*nRFjprog_disable_bprot)                     (void);
typedef nrfjprogdll_err_t (*nRFjprog_erase_all)                         (void);
typedef nrfjprogdll_err_t (*nRFjprog_erase_page)                        (uint32_t addr);
typedef nrfjprogdll_err_t (*nRFjprog_erase_uicr)                        (void);
typedef nrfjprogdll_err_t (*nRFjprog_write_u32)                         (uint32_t addr, uint32_t data, bool nvmc_control);
typedef nrfjprogdll_err_t (*nRFjprog_read_u32)                          (uint32_t addr, uint32_t * data);
typedef nrfjprogdll_err_t (*nRFjprog_write)                             (uint32_t addr, const uint8_t * data, uint32_t data_len, bool nvmc_control);
typedef nrfjprogdll_err_t (*nRFjprog_read)                              (uint32_t addr, uint8_t * data, uint32_t data_len);
typedef nrfjprogdll_err_t (*nRFjprog_is_halted)                         (bool * is_device_halted);
typedef nrfjprogdll_err_t (*nRFjprog_halt)                              (void);
typedef nrfjprogdll_err_t (*nRFjprog_run)                               (uint32_t pc, uint32_t sp);
typedef nrfjprogdll_err_t (*nRFjprog_go)                                (void);
typedef nrfjprogdll_err_t (*nRFjprog_step)                              (void);
typedef nrfjprogdll_err_t (*nRFjprog_read_ram_sections_count)           (uint32_t * ram_sections_count);
typedef nrfjprogdll_err_t (*nRFjprog_read_ram_sections_size)            (uint32_t * ram_sections_size, uint32_t ram_sections_size_len);
typedef nrfjprogdll_err_t (*nRFjprog_read_ram_sections_power_status)    (ram_section_power_status_t * ram_sections_power_status, uint32_t ram_sections_power_status_len);
typedef nrfjprogdll_err_t (*nRFjprog_is_ram_powered)                    (ram_section_power_status_t * ram_sections_power_status, uint32_t ram_sections_power_status_array_size, uint32_t * ram_sections_number, uint32_t * ram_sections_size);
typedef nrfjprogdll_err_t (*nRFjprog_power_ram_all)                     (void);
typedef nrfjprogdll_err_t (*nRFjprog_unpower_ram_section)               (uint32_t section_index);
typedef nrfjprogdll_err_t (*nRFjprog_read_cpu_register)                 (cpu_registers_t register_name, uint32_t * register_value);
typedef nrfjprogdll_err_t (*nRFjprog_write_cpu_register)                (cpu_registers_t register_name, uint32_t register_value);
typedef nrfjprogdll_err_t (*nRFjprog_read_device_version)               (device_version_t * version);
typedef nrfjprogdll_err_t (*nRFjprog_read_device_family)                (device_family_t * family);
typedef nrfjprogdll_err_t (*nRFjprog_read_debug_port_register)          (uint8_t reg_addr, uint32_t * data);
typedef nrfjprogdll_err_t (*nRFjprog_write_debug_port_register)         (uint8_t reg_addr, uint32_t data);
typedef nrfjprogdll_err_t (*nRFjprog_read_access_port_register)         (uint8_t ap_index, uint8_t reg_addr, uint32_t * data);
typedef nrfjprogdll_err_t (*nRFjprog_write_access_port_register)        (uint8_t ap_index, uint8_t reg_addr, uint32_t data);
typedef nrfjprogdll_err_t (*nRFjprog_is_rtt_started)                    (bool * started);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_set_control_block_address)     (uint32_t address);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_start)                         (void);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_is_control_block_found)        (bool * is_control_block_found);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_stop)                          (void);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_read)                          (uint32_t up_channel_index, char * data, uint32_t data_len, uint32_t * data_read);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_write)                         (uint32_t down_channel_index, const char * data, uint32_t data_len, uint32_t * data_written);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_read_channel_count)            (uint32_t * down_channel_number, uint32_t * up_channel_number);
typedef nrfjprogdll_err_t (*nRFjprog_rtt_read_channel_info)             (uint32_t channel_index, rtt_direction_t dir, char * channel_name, uint32_t channel_name_len, uint32_t * channel_size);
typedef nrfjprogdll_err_t (*nRFjprog_is_qspi_init)                      (bool * initialized);
typedef nrfjprogdll_err_t (*nRFjprog_qspi_init)                         (bool retain_ram, const qspi_init_params_t * init_params);
typedef nrfjprogdll_err_t (*nRFjprog_qspi_uninit)                       (void);
typedef nrfjprogdll_err_t (*nRFjprog_qspi_read)                         (uint32_t addr, uint8_t * data, uint32_t data_len);
typedef nrfjprogdll_err_t (*nRFjprog_qspi_write)                        (uint32_t addr, const uint8_t * data, uint32_t data_len);
typedef nrfjprogdll_err_t (*nRFjprog_qspi_erase)                        (uint32_t addr, qspi_erase_len_t length);
typedef nrfjprogdll_err_t (*nRFjprog_qspi_custom)                       (uint8_t instruction_code, uint8_t instruction_length, const uint8_t * data_in, uint8_t * data_out);

struct nRFjprogDllFunctionPointersType {
    nRFjprog_dll_version                      dll_version;
    nRFjprog_is_dll_open                      is_dll_open;
    nRFjprog_open_dll                         open_dll;
    nRFjprog_close_dll                        close_dll;
    nRFjprog_enum_emu_snr                     enum_emu_snr;
    nRFjprog_is_connected_to_emu              is_connected_to_emu;
    nRFjprog_connect_to_emu_with_snr          connect_to_emu_with_snr;
    nRFjprog_connect_to_emu_without_snr       connect_to_emu_without_snr;
    nRFjprog_read_connected_emu_snr           read_connected_emu_snr;
    nRFjprog_read_connected_emu_fwstr         read_connected_emu_fwstr;
    nRFjprog_disconnect_from_emu              disconnect_from_emu;
    nRFjprog_recover                          recover;
    nRFjprog_is_connected_to_device           is_connected_to_device;
    nRFjprog_connect_to_device                connect_to_device;
    nRFjprog_disconnect_from_device           disconnect_from_device;
    nRFjprog_readback_protect                 readback_protect;
    nRFjprog_readback_status                  readback_status;
    nRFjprog_read_region_0_size_and_source    read_region_0_size_and_source;
    nRFjprog_debug_reset                      debug_reset;
    nRFjprog_sys_reset                        sys_reset;
    nRFjprog_pin_reset                        pin_reset;
    nRFjprog_disable_bprot                    disable_bprot;
    nRFjprog_erase_all                        erase_all;
    nRFjprog_erase_page                       erase_page;
    nRFjprog_erase_uicr                       erase_uicr;
    nRFjprog_write_u32                        write_u32;
    nRFjprog_read_u32                         read_u32;
    nRFjprog_write                            write;
    nRFjprog_read                             read;
    nRFjprog_is_halted                        is_halted;
    nRFjprog_halt                             halt;
    nRFjprog_run                              run;
    nRFjprog_go                               go;
    nRFjprog_step                             step;
    nRFjprog_read_ram_sections_count          read_ram_sections_count;
    nRFjprog_read_ram_sections_size           read_ram_sections_size;
    nRFjprog_read_ram_sections_power_status   read_ram_sections_power_status;
    nRFjprog_is_ram_powered                   is_ram_powered;
    nRFjprog_power_ram_all                    power_ram_all;
    nRFjprog_unpower_ram_section              unpower_ram_section;
    nRFjprog_read_cpu_register                read_cpu_register;
    nRFjprog_write_cpu_register               write_cpu_register;
    nRFjprog_read_device_version              read_device_version;
    nRFjprog_read_device_family               read_device_family;
    nRFjprog_read_debug_port_register         read_debug_port_register;
    nRFjprog_write_debug_port_register        write_debug_port_register;
    nRFjprog_read_access_port_register        read_access_port_register;
    nRFjprog_write_access_port_register       write_access_port_register;
    nRFjprog_is_rtt_started                   is_rtt_started;
    nRFjprog_rtt_set_control_block_address    rtt_set_control_block_address;
    nRFjprog_rtt_start                        rtt_start;
    nRFjprog_rtt_is_control_block_found       rtt_is_control_block_found;
    nRFjprog_rtt_stop                         rtt_stop;
    nRFjprog_rtt_read                         rtt_read;
    nRFjprog_rtt_write                        rtt_write;
    nRFjprog_rtt_read_channel_count           rtt_read_channel_count;
    nRFjprog_rtt_read_channel_info            rtt_read_channel_info;
    nRFjprog_is_qspi_init                     is_qspi_init;
    nRFjprog_qspi_init                        qspi_init;
    nRFjprog_qspi_uninit                      qspi_uninit;
    nRFjprog_qspi_read                        qspi_read;
    nRFjprog_qspi_write                       qspi_write;
    nRFjprog_qspi_erase                       qspi_erase;
    nRFjprog_qspi_custom                      qspi_custom;
};

errorcode_t loadnRFjprogFunctions(const char * path, nRFjprogDllFunctionPointersType * dll_function);
errorcode_t releasenRFjprog();

#endif
