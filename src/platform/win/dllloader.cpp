#include "../../dllloader.h"

#include <windows.h>

LoadedFunctionType LoadFunction(LibraryHandleType dll_handle, const char *func_name)
{
    return GetProcAddress(dll_handle, func_name);
}

LibraryHandleType DllLoad(const char *path)
{
    return LoadLibrary(path);
}

void DllFree(LibraryHandleType dll)
{
    if (dll) {
        FreeLibrary(dll);
    }
}
