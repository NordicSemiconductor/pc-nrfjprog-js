#ifndef DLL_FUNC_PTRS_H
#define DLL_FUNC_PTRS_H

#include "nrfjprogdll.h"

typedef nrfjprogdll_err_t   (*Dll_dll_version_t)                    (uint32_t * major, uint32_t * minor, char * revision);

typedef nrfjprogdll_err_t   (*Dll_open_dll_t)                       (const char * jlink_path, msg_callback * log_cb, device_family_t family);
typedef void                (*Dll_close_dll_t)                      (void);

typedef nrfjprogdll_err_t   (*Dll_enum_emu_snr_t)                   (uint32_t serial_numbers[], uint32_t serial_numbers_len, uint32_t * num_available);

typedef nrfjprogdll_err_t   (*Dll_connect_to_emu_with_snr_t)        (uint32_t serial_number, uint32_t clock_speed_in_khz);
typedef nrfjprogdll_err_t   (*Dll_connect_to_emu_without_snr_t)     (uint32_t clock_speed_in_khz);
typedef nrfjprogdll_err_t   (*Dll_disconnect_from_emu_t)            (void);

typedef nrfjprogdll_err_t   (*Dll_recover_t)                        (void);

typedef nrfjprogdll_err_t   (*Dll_connect_to_device_t)              (void);

typedef nrfjprogdll_err_t   (*Dll_readback_protect_t)               (readback_protection_status_t desired_protection);
typedef nrfjprogdll_err_t   (*Dll_readback_status_t)                (readback_protection_status_t * status);
typedef nrfjprogdll_err_t   (*Dll_read_region_0_size_and_source_t)  (uint32_t * size, region_0_source_t * source);

typedef nrfjprogdll_err_t   (*Dll_debug_reset_t)                    (void);
typedef nrfjprogdll_err_t   (*Dll_sys_reset_t)                      (void);
typedef nrfjprogdll_err_t   (*Dll_pin_reset_t)                      (void);

typedef nrfjprogdll_err_t   (*Dll_disable_bprot_t)                  (void);
typedef nrfjprogdll_err_t   (*Dll_erase_all_t)                      (void);
typedef nrfjprogdll_err_t   (*Dll_erase_page_t)                     (uint32_t addr);
typedef nrfjprogdll_err_t   (*Dll_erase_uicr_t)                     (void);

typedef nrfjprogdll_err_t   (*Dll_write_u32_t)                      (uint32_t addr, uint32_t data, bool nvmc_control);
typedef nrfjprogdll_err_t   (*Dll_read_u32_t)                       (uint32_t addr, uint32_t * data);
typedef nrfjprogdll_err_t   (*Dll_write_t)                          (uint32_t addr, const uint8_t * data, uint32_t data_len, bool nvmc_control);
typedef nrfjprogdll_err_t   (*Dll_read_t)                           (uint32_t addr, uint8_t * data, uint32_t data_len);

typedef nrfjprogdll_err_t   (*Dll_masserase_t)                      (void);
typedef nrfjprogdll_err_t   (*Dll_ficrwrite_u32_t)                  (uint32_t addr, uint32_t data);
typedef nrfjprogdll_err_t   (*Dll_ficrwrite_t)                      (uint32_t addr, const uint8_t * data, uint32_t data_len);

typedef nrfjprogdll_err_t   (*Dll_is_halted_t)                      (bool * is_halted);
typedef nrfjprogdll_err_t   (*Dll_halt_t)                           (void);
typedef nrfjprogdll_err_t   (*Dll_run_t)                            (uint32_t pc, uint32_t sp);
typedef nrfjprogdll_err_t   (*Dll_go_t)                             (void);

typedef nrfjprogdll_err_t   (*Dll_is_ram_powered_t)                 (ram_section_power_status_t * ram_sections_power_status, uint32_t ram_sections_power_status_array_size, uint32_t * ram_sections_number, uint32_t * ram_sections_size);
typedef nrfjprogdll_err_t   (*Dll_power_ram_all_t)                  (void);

typedef nrfjprogdll_err_t   (*Dll_read_cpu_register_t)              (cpu_registers_t register_name, uint32_t * register_value);

typedef nrfjprogdll_err_t   (*Dll_read_device_version_t)            (device_version_t * version);


struct DllFunctionPointersType {
    
    Dll_dll_version_t                                       get_dll_version;
    
    Dll_open_dll_t                                          open_dll;
    Dll_close_dll_t                                         close_dll;
    
    Dll_enum_emu_snr_t                                      enum_emu_snr;
    Dll_connect_to_emu_with_snr_t                           connect_to_emu_with_snr;
    Dll_connect_to_emu_without_snr_t                        connect_to_emu_without_snr;

    Dll_disconnect_from_emu_t                               disconnect_from_emu;
    
    Dll_recover_t                                           recover;

    Dll_connect_to_device_t                                 connect_to_device;
    
    Dll_readback_protect_t                                  readback_protect;
    Dll_readback_status_t                                   readback_status;
    Dll_read_region_0_size_and_source_t                     read_region_0_size_and_source;
    
    Dll_debug_reset_t                                       debug_reset;    
    Dll_sys_reset_t                                         sys_reset;
    Dll_pin_reset_t                                         pin_reset;
    
    Dll_disable_bprot_t                                     disable_bprot;
    Dll_erase_all_t                                         erase_all;
    Dll_erase_page_t                                        erase_page;
    Dll_erase_uicr_t                                        erase_uicr;
    
    Dll_write_u32_t                                         write_u32;
    Dll_read_u32_t                                          read_u32;
    Dll_write_t                                             write;
    Dll_read_t                                              read;

    Dll_masserase_t                                         masserase;
    Dll_ficrwrite_u32_t                                     ficrwrite_u32;
    Dll_ficrwrite_t                                         ficrwrite;

    Dll_is_halted_t                                         is_halted;
    Dll_halt_t                                              halt;
    Dll_run_t                                               run;
    Dll_go_t                                                go;

    Dll_is_ram_powered_t                                    is_ram_powered;
    Dll_power_ram_all_t                                     power_ram_all;

    Dll_read_cpu_register_t                                 read_cpu_register;

    Dll_read_device_version_t                               read_device_version;
};


NrfjprogErrorCodesType DllLoad(const char * path, DllFunctionPointersType * dll_function);

void DllFree(void);


#endif
