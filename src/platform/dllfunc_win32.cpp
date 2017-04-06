
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

    if (!load_func_ptr(&dll_function->dll_get_version, "HiLvlnRFJ_dll_get_version", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->dll_open, "HiLvlnRFJ_dll_open", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->dll_close, "HiLvlnRFJ_dll_close", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->is_dll_open, "HiLvlnRFJ_is_dll_open", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->get_connected_probes, "HiLvlnRFJ_get_connected_probes", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->probe_init, "HiLvlnRFJ_probe_init", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->probe_uninit, "HiLvlnRFJ_probe_uninit", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->probe_get_snr, "HiLvlnRFJ_probe_get_snr", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->get_device_family, "HiLvlnRFJ_get_device_family", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->get_device_version, "HiLvlnRFJ_get_device_version", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->program, "HiLvlnRFJ_program", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->read_to_file, "HiLvlnRFJ_read_to_file", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->erase, "HiLvlnRFJ_erase", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->recover, "HiLvlnRFJ_recover", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->read, "HiLvlnRFJ_read", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->read_u32, "HiLvlnRFJ_read_u32", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->write, "HiLvlnRFJ_write", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->write_u32, "HiLvlnRFJ_write_u32", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->reset, "HiLvlnRFJ_reset", dll)) {
        return NrfjprogDllFunctionLoadFailedError;
    }

    if (!load_func_ptr(&dll_function->go, "HiLvlnRFJ_go", dll)) {
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
