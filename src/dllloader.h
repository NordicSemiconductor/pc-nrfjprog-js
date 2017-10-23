#ifndef __DLL_LOADER_H__
#define __DLL_LOADER_H__

#include "dllloader_platform.h"

LoadedFunctionType LoadFunction(LibraryHandleType, const char *func_name);

template<typename T>
static bool load_func_ptr(T * func_ptr, const char * func_name, LibraryHandleType dll_handle)
{
    *func_ptr = (T) LoadFunction(dll_handle, func_name);

    if (*func_ptr == NULL) {
        return false;
    }

    return true;
}

LibraryHandleType DllLoad(const char *path);
void DllFree(LibraryHandleType dll_handle);

#endif
