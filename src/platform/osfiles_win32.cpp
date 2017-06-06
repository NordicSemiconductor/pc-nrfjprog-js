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

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

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

NrfjprogErrorCodesType OSFilesFindDllByHKey(const HKEY rootKey, char * dll_path, int dll_path_len)
{
    HKEY key;
    HKEY innerKey;

    CHAR install_path[COMMON_MAX_PATH] = {'\0'};
    DWORD install_path_size = sizeof(install_path);

    /* Search for JLinkARM in the Local Machine Key.  */
    if (RegOpenKeyEx(rootKey, "Software\\Nordic Semiconductor\\nrfjprog", 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &key) == ERROR_SUCCESS)
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
            strncat(dll_path, "highlevelnrfjprog.dll", dll_path_len - strlen(dll_path) - 1);
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

    return NrfjprogDllNotFoundError;
}

NrfjprogErrorCodesType OSFilesFindDll(char * dll_path, int dll_path_len)
{
    NrfjprogErrorCodesType retCode = OSFilesFindDllByHKey(HKEY_LOCAL_MACHINE, dll_path, dll_path_len);

    if (retCode != Success) {
        retCode = OSFilesFindDllByHKey(HKEY_CURRENT_USER, dll_path, dll_path_len);
    }

    return retCode;
}
