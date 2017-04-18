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
 
#include <dlfcn.h>
#include <stddef.h>

#include "nrfjprog.h"
#include "../dllfunc.h"

static void * dll = NULL;

template<typename T>
static bool load_func_ptr(T * func_ptr, const char * func_name, void * dll_handle)
{
    *func_ptr = (T) dlsym(dll_handle, func_name);
    if (*func_ptr == NULL){
        return false;
    }
    return true;
}


NrfjprogErrorCodesType DllLoad(const char * path, DllFunctionPointersType * dll_function)
{
    dll = dlopen(path, RTLD_LAZY);
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
        dlclose(dll);
    }
}
