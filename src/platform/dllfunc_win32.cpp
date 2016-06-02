
#include <windows.h>

#include "nrfjprog.h"
#include "../dllfunc.h"

static HMODULE dll = NULL;

template<typename T>
static bool load_func_ptr(T * func_ptr, const char * func_name, HMODULE dll_handle)
{
    *func_ptr = (T) GetProcAddress(dll_handle, func_name);
    if (*func_ptr == NULL){
        return false;
    }
    return true;
}


NrfjprogErrorCodesType DllLoad(const char * path, DllFunctionPointersType * dll_function)
{
    dll = LoadLibrary(path);
    if (!dll){
        return NrfjprogDllLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->get_dll_version, "NRFJPROG_dll_version", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    
    if (!load_func_ptr(&dll_function->open_dll, "NRFJPROG_open_dll", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    if (!load_func_ptr(&dll_function->close_dll, "NRFJPROG_close_dll", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }

    if (!load_func_ptr(&dll_function->enum_emu_snr, "NRFJPROG_enum_emu_snr", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }

    if (!load_func_ptr(&dll_function->connect_to_emu_with_snr, "NRFJPROG_connect_to_emu_with_snr", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }

    if (!load_func_ptr(&dll_function->connect_to_emu_without_snr, "NRFJPROG_connect_to_emu_without_snr", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }

    if (!load_func_ptr(&dll_function->disconnect_from_emu, "NRFJPROG_disconnect_from_emu", dll))
    {
        return NrfjprogDllFunctionLoadFailedError;
    }
    
    if (!load_func_ptr(&dll_function->recover, "NRFJPROG_recover", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }

    if (!load_func_ptr(&dll_function->connect_to_device, "NRFJPROG_connect_to_device", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }

    if (!load_func_ptr(&dll_function->readback_protect, "NRFJPROG_readback_protect", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    if (!load_func_ptr(&dll_function->readback_status, "NRFJPROG_readback_status", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    if (!load_func_ptr(&dll_function->read_region_0_size_and_source, "NRFJPROG_read_region_0_size_and_source", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }

    if (!load_func_ptr(&dll_function->debug_reset, "NRFJPROG_debug_reset", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    if (!load_func_ptr(&dll_function->sys_reset, "NRFJPROG_sys_reset", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    if (!load_func_ptr(&dll_function->pin_reset, "NRFJPROG_pin_reset", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    
    if (!load_func_ptr(&dll_function->disable_bprot, "NRFJPROG_disable_bprot", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    if (!load_func_ptr(&dll_function->erase_all, "NRFJPROG_erase_all", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    if (!load_func_ptr(&dll_function->erase_page, "NRFJPROG_erase_page", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }
    if (!load_func_ptr(&dll_function->erase_uicr, "NRFJPROG_erase_uicr", dll)){ 
        return NrfjprogDllFunctionLoadFailedError; 
    }

    if (!load_func_ptr(&dll_function->write_u32, "NRFJPROG_write_u32", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }
    if (!load_func_ptr(&dll_function->read_u32, "NRFJPROG_read_u32", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }
    if (!load_func_ptr(&dll_function->write, "NRFJPROG_write", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }
    if (!load_func_ptr(&dll_function->read, "NRFJPROG_read", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->masserase, "NRFJPROG_masserase", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }
    if (!load_func_ptr(&dll_function->ficrwrite_u32, "NRFJPROG_ficrwrite_u32", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }
    if (!load_func_ptr(&dll_function->ficrwrite, "NRFJPROG_ficrwrite", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->is_halted, "NRFJPROG_is_halted", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }
    if (!load_func_ptr(&dll_function->halt, "NRFJPROG_halt", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }
    if (!load_func_ptr(&dll_function->run, "NRFJPROG_run", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }
    if (!load_func_ptr(&dll_function->go, "NRFJPROG_go", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->is_ram_powered, "NRFJPROG_is_ram_powered", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }
    if (!load_func_ptr(&dll_function->power_ram_all, "NRFJPROG_power_ram_all", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->read_cpu_register, "NRFJPROG_read_cpu_register", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->read_device_version, "NRFJPROG_read_device_version", dll)){ 
        return NrfjprogDllFunctionLoadFailedError;
    }

    return Success;
}


void DllFree(void)
{
    if (dll){
        FreeLibrary(dll);
    }
}
