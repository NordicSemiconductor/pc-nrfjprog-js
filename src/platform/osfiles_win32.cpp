
#define _CRT_SECURE_NO_WARNINGS

#include "Shlwapi.h"
#include <windows.h>


#include "nrfjprog.h"
#include "../osfiles.h"

#include <iostream>

#pragma comment(lib, "Shlwapi.lib")


bool OSFilesExists(char * path)
{
     return PathFileExists(path) == TRUE;
}

#define MAX_KEY_LENGTH 1000
#define MAX_VALUE_NAME 1000

NrfjprogErrorCodesType OSFilesFindDll(char * dll_path, int dll_path_len)
{
    HKEY key;
    HKEY innerKey;

    CHAR install_path[COMMON_MAX_PATH] = {'\0'};
    DWORD install_path_size = sizeof(install_path);

    /* Search for JLinkARM in the Local Machine Key.  */
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Nordic Semiconductor\\nrfjprog", 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &key) == ERROR_SUCCESS)
    {
        TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
        DWORD    cbName;                   // size of name string
        TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name
        DWORD    cchClassName = MAX_PATH;  // size of class string
        DWORD    cSubKeys = 0;               // number of subkeys
        DWORD    cbMaxSubKey;              // longest subkey size
        DWORD    cchMaxClass;              // longest class string
        DWORD    cValues;              // number of values for key
        DWORD    cchMaxValue;          // longest value name
        DWORD    cbMaxValueData;       // longest value data
        DWORD    cbSecurityDescriptor; // size of security descriptor
        FILETIME ftLastWriteTime;      // last write time

        DWORD retCode;

        TCHAR  achValue[MAX_VALUE_NAME];
        DWORD cchValue = MAX_VALUE_NAME;

        retCode = RegQueryInfoKey(
            key,                    // key handle
            achClass,                // buffer for class name
            &cchClassName,           // size of class string
            NULL,                    // reserved
            &cSubKeys,               // number of subkeys
            &cbMaxSubKey,            // longest subkey size
            &cchMaxClass,            // longest class string
            &cValues,                // number of values for this key
            &cchMaxValue,            // longest value name
            &cbMaxValueData,         // longest value data
            &cbSecurityDescriptor,   // security descriptor
            &ftLastWriteTime);       // last write time

        // Enumerate the subkeys, until RegEnumKeyEx fails.

        if (cSubKeys)
        {
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(key,
                                    cSubKeys - 1,
                                    achKey,
                                    &cbName,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &ftLastWriteTime);
            RegOpenKeyEx(key, achKey, 0, KEY_QUERY_VALUE, &innerKey);
        }

        /* If it is found, read the install path. */
        if (RegQueryValueEx(innerKey, "InstallPath", NULL, NULL, (LPBYTE)&install_path, &install_path_size) == ERROR_SUCCESS)
        {
            /* Copy, check it exists and return if it does. */
            strncpy(dll_path, install_path, dll_path_len);
            strncat(dll_path, "nrfjprog.dll", dll_path_len - strlen(dll_path) - 1);
            RegCloseKey(innerKey);
            RegCloseKey(key);
            if (OSFilesExists(dll_path))
            {
                return Success;
            }
        }

        /* In case we did not obtain the path, for whatever reason, we close the key. */
        RegCloseKey(innerKey);
        RegCloseKey(key);
    }

    /* Search for JLinkARM in the Current User Key.  */
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Nordic Semiconductor\\nrfjprog", 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &key) == ERROR_SUCCESS)
    {
        TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
        DWORD    cbName;                   // size of name string
        TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name
        DWORD    cchClassName = MAX_PATH;  // size of class string
        DWORD    cSubKeys = 0;               // number of subkeys
        DWORD    cbMaxSubKey;              // longest subkey size
        DWORD    cchMaxClass;              // longest class string
        DWORD    cValues;              // number of values for key
        DWORD    cchMaxValue;          // longest value name
        DWORD    cbMaxValueData;       // longest value data
        DWORD    cbSecurityDescriptor; // size of security descriptor
        FILETIME ftLastWriteTime;      // last write time

        DWORD retCode;

        TCHAR  achValue[MAX_VALUE_NAME];
        DWORD cchValue = MAX_VALUE_NAME;

        retCode = RegQueryInfoKey(
            key,                    // key handle
            achClass,                // buffer for class name
            &cchClassName,           // size of class string
            NULL,                    // reserved
            &cSubKeys,               // number of subkeys
            &cbMaxSubKey,            // longest subkey size
            &cchMaxClass,            // longest class string
            &cValues,                // number of values for this key
            &cchMaxValue,            // longest value name
            &cbMaxValueData,         // longest value data
            &cbSecurityDescriptor,   // security descriptor
            &ftLastWriteTime);       // last write time

        // Enumerate the subkeys, until RegEnumKeyEx fails.

        if (cSubKeys)
        {
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(key,
                                   cSubKeys - 1,
                                   achKey,
                                   &cbName,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &ftLastWriteTime);
            RegOpenKeyEx(key, achKey, 0, KEY_QUERY_VALUE, &innerKey);
        }

        /* If it is found, read the install path. */
        if (RegQueryValueEx(innerKey, "InstallPath", NULL, NULL, (LPBYTE)&install_path, &install_path_size) == ERROR_SUCCESS)
        {
            /* Copy, check it exists and return if it does. */
            strncpy(dll_path, install_path, dll_path_len);
            strncat(dll_path, "nrfjprog.dll", dll_path_len - strlen(dll_path) - 1);
            RegCloseKey(innerKey);
            RegCloseKey(key);
            if (OSFilesExists(dll_path))
            {
                return Success;
            }
        }

        /* In case we did not obtain the path, for whatever reason, we close the key. */
        RegCloseKey(innerKey);
        RegCloseKey(key);
    }

    return NrfjprogDllNotFoundError;
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
            strncat(jlink_path, "JLinkARM.dll", jlink_path_len - strlen(jlink_path) - 1);
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
            strncat(jlink_path, "JLinkARM.dll", jlink_path_len - strlen(jlink_path) - 1);
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
