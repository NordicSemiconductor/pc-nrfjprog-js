
#define _CRT_SECURE_NO_WARNINGS

#include "Shlwapi.h"
#include <windows.h>


#include "nrfjprog.h"
#include "../osfiles.h"

#pragma comment(lib, "Shlwapi.lib")


bool OSFilesExists(char * path)
{
     return PathFileExists(path) == TRUE;
}


NrfjprogErrorCodesType OSFilesFindDll(char * dll_path, int dll_path_len)
{
    GetModuleFileName(NULL, dll_path, dll_path_len);
    PathRemoveFileSpec(dll_path);
    strncat(dll_path, "\\nrfjprog.dll", dll_path_len - strlen(dll_path) - 1);

    if (!OSFilesExists(dll_path)) {
        return NrfjprogDllNotFoundError;
    }
    return Success;
}   

NrfjprogErrorCodesType OSFilesFindJLink(char * jlink_path, int jlink_path_len)
{
    HKEY key;
    
    CHAR install_path[COMMON_MAX_PATH] = {'\0'};
    DWORD install_path_size = sizeof(install_path);

    /* Search for JLinkARM in the Local Machine Key.  */
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\SEGGER\\J-Link", 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
    {
        
        /* If it is found, read the install path. */ 
        if (RegQueryValueEx(key, "InstallPath", NULL, NULL, (LPBYTE)&install_path, &install_path_size) == ERROR_SUCCESS)
        {
            /* Copy, check it exists and return if it does. */
            strncpy(jlink_path, install_path, jlink_path_len);
            strncat(jlink_path, "\\JLinkARM.dll", jlink_path_len - strlen(jlink_path) - 1);
            RegCloseKey(key);
            if (OSFilesExists(jlink_path)){
                return Success;
            }
        }
             
        /* In case we did not obtain the path, for whatever reason, we close the key. */
        RegCloseKey(key);  
    }

    /* Search for JLinkARM in the Current User Key.  */
    if(RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\SEGGER\\J-Link", 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
    {
        
        /* If it is found, read the install path. */ 
        if (RegQueryValueEx(key, "InstallPath", NULL, NULL, (LPBYTE)&install_path, &install_path_size) == ERROR_SUCCESS)
        {
            /* Copy, check it exists and return if it does. */
            strncpy(jlink_path, install_path, jlink_path_len);
            strncat(jlink_path, "\\JLinkARM.dll", jlink_path_len - strlen(jlink_path) - 1);
            RegCloseKey(key);
            if (OSFilesExists(jlink_path)){
                return Success;
            }
        }
             
        /* In case we did not obtain the path, for whatever reason, we close the key. */
        RegCloseKey(key);  
    }

    return JLinkARMDllNotFoundError;
}

NrfjprogErrorCodesType OSFilesFindIni(char * ini_path, int ini_path_len) 
{
    GetModuleFileName(NULL, ini_path, ini_path_len);
    PathRemoveFileSpec(ini_path);
    strncat(ini_path, "\\nrfjprog.ini", ini_path_len - strlen(ini_path) - 1);

    if (!OSFilesExists(ini_path)){
        return NrfjprogIniNotFoundError;
    }
    return Success;
}




