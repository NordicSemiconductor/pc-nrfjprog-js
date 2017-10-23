#include "dllfunc.h"
#include "dllloader.h"

LibraryHandleType dll;

errorcode_t loadFunctions(const char * path, DllFunctionPointersType * dll_function)
{
    dll = DllLoad(path);

    if (!dll){
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->dll_get_version, "NRFJPROG_dll_version", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->dll_open, "NRFJPROG_dll_open", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->dll_close, "NRFJPROG_dll_close", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->is_dll_open, "NRFJPROG_is_dll_open", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->get_connected_probes, "NRFJPROG_get_connected_probes", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->probe_init, "NRFJPROG_probe_init", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->probe_uninit, "NRFJPROG_probe_uninit", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->get_library_info, "NRFJPROG_get_library_info", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->get_probe_info, "NRFJPROG_get_probe_info", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->get_device_info, "NRFJPROG_get_device_info", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->program, "NRFJPROG_program", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_to_file, "NRFJPROG_read_to_file", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->verify, "NRFJPROG_verify", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->erase, "NRFJPROG_erase", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->recover, "NRFJPROG_recover", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read, "NRFJPROG_read", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->read_u32, "NRFJPROG_read_u32", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->write, "NRFJPROG_write", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->write_u32, "NRFJPROG_write_u32", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->reset, "NRFJPROG_reset", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&dll_function->run, "NRFJPROG_run", dll)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    return errorcode_t::JsSuccess;
}

errorcode_t release() {
    DllFree(dll);
    return errorcode_t::JsSuccess;
}
