/* Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
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

#include <iostream>

#include "../../osfiles.h"
#include "../../utility/conversion.h"

#include <nan.h>

#include "Shlwapi.h"
#include <windows.h>

#pragma comment(lib, "Shlwapi.lib")

#define MAX_KEY_LENGTH 1000
#define MAX_VALUE_NAME 1000

std::string *pLibrarySearchPath = nullptr;

NAN_METHOD(OSFilesSetLibrarySearchPath)
{
    static std::string librarySearchPath;
    pLibrarySearchPath = &librarySearchPath;

    // Parse parameter from the FunctionCallbackInfo received
    if (info.Length() > 0 && info[0]->IsString())
    {
        librarySearchPath.assign(Convert::getNativeString(info[0]));
    }
    else
    {
        Nan::ThrowError(Nan::New("Expected string as the first argument").ToLocalChecked());
    }
}

errorcode_t OSFilesFindLibraryByHKey(HKEY rootKey, std::string &libraryPath,
                                     const std::string &fileName)
{
    HKEY key;
    HKEY innerKey;

    std::vector<char> installPath(COMMON_MAX_PATH, '\0');
    DWORD installPathSize = installPath.size();

    /* Search for JLinkARM in the Local Machine Key.  */
    if (RegOpenKeyEx(rootKey, "Software\\Nordic Semiconductor\\nrfjprog", 0,
                     static_cast<uint32_t>(KEY_QUERY_VALUE) |
                         static_cast<uint32_t>(KEY_ENUMERATE_SUB_KEYS),
                     &key) == ERROR_SUCCESS)
    {
        std::vector<TCHAR> achKey(MAX_KEY_LENGTH, '\0'); // buffer for subkey name
        DWORD cbName;                                    // size of name string
        std::vector<TCHAR> achClass(MAX_PATH, '\0');     // buffer for class name
        DWORD cchClassName = MAX_PATH;                   // size of class string
        DWORD cSubKeys     = 0u;                         // number of subkeys
        DWORD cbMaxSubKey;                               // longest subkey size
        DWORD cchMaxClass;                               // longest class string
        DWORD cValues;                                   // number of values for key
        DWORD cchMaxValue;                               // longest value name
        DWORD cbMaxValueData;                            // longest value data
        DWORD cbSecurityDescriptor;                      // size of security descriptor
        FILETIME ftLastWriteTime;                        // last write time

        RegQueryInfoKey(key,                   // key handle
                        achClass.data(),       // buffer for class name
                        &cchClassName,         // size of class string
                        nullptr,               // reserved
                        &cSubKeys,             // number of subkeys
                        &cbMaxSubKey,          // longest subkey size
                        &cchMaxClass,          // longest class string
                        &cValues,              // number of values for this key
                        &cchMaxValue,          // longest value name
                        &cbMaxValueData,       // longest value data
                        &cbSecurityDescriptor, // security descriptor
                        &ftLastWriteTime);     // last write time

        // Enumerate the subkeys, until RegEnumKeyEx fails.
        if (cSubKeys != 0u)
        {
            cbName = MAX_KEY_LENGTH;
            if (RegEnumKeyEx(key, cSubKeys - 1, achKey.data(), &cbName, nullptr, nullptr, nullptr,
                             &ftLastWriteTime) == ERROR_SUCCESS)
            {
                if (RegOpenKeyEx(key, achKey.data(), 0, KEY_QUERY_VALUE, &innerKey) ==
                    ERROR_SUCCESS)
                {
                    /* If it is found, read the install path. */
                    if (RegQueryValueEx(innerKey, "InstallPath", nullptr, nullptr,
                                        reinterpret_cast<LPBYTE>(installPath.data()),
                                        &installPathSize) == ERROR_SUCCESS)
                    {
                        /* Copy, check it exists and return if it does. */
                        libraryPath.assign(installPath.data());
                        libraryPath.append(fileName);
                        RegCloseKey(innerKey);
                        RegCloseKey(key);
                        if (TempFile::pathExists(libraryPath.c_str()))
                        {
                            return errorcode_t::JsSuccess;
                        }
                    }

                    /* In case we did not obtain the path, for whatever reason, we close the key. */
                    RegCloseKey(innerKey);
                }
            }
        }
        RegCloseKey(key);
    }

    /* Search for JLinkARM in the Current User Key.  */
    return errorcode_t::CouldNotFindJprogDLL;
}

errorcode_t OSFilesFindLibrary(std::string &libraryPath, const std::string &fileName)
{
    // Try to find the DLLs from the path given to OSFilesSetLibrarySearchPath()
    if (pLibrarySearchPath != nullptr)
    {
        libraryPath.assign(*pLibrarySearchPath);
    }
    else
    {
        libraryPath.assign("");
    }
    libraryPath.append("\\");
    libraryPath.append(fileName);
    if (AbstractFile::pathExists(libraryPath))
    {
        return errorcode_t::JsSuccess;
    }

    // If that fails, try to find the DLLs when installed in the whole machine (for all users)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    errorcode_t retCode = OSFilesFindLibraryByHKey(HKEY_LOCAL_MACHINE, libraryPath, fileName);
    if (retCode == errorcode_t::JsSuccess)
    {
        return retCode;
    }

    // If that fails, try to find the DLLs when installed for the current user only
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    retCode = OSFilesFindLibraryByHKey(HKEY_CURRENT_USER, libraryPath, fileName);

    return retCode;
}

std::string TempFile::concatPaths(const std::string &basePath, const std::string &relativePath)
{
    std::vector<char> buffer(MAX_PATH);

    PathCombine(buffer.data(), basePath.c_str(), relativePath.c_str());

    return std::string(buffer.data());
}

bool AbstractFile::pathExists(const char *path)
{
    return PathFileExists(path) == TRUE;
}

std::string TempFile::getTempFileName()
{
    /* Folder name should never be longer than MAX_PATH-14 characters to be compatible with
     * GetTempFileName function. */
    std::vector<char> tempFolderPath(MAX_PATH);
    std::vector<char> tempFilePath(MAX_PATH);

    DWORD pathLength = GetTempPath(MAX_PATH, tempFolderPath.data());

    if (pathLength > MAX_PATH || (pathLength == 0))
    {
        error = TempPathNotFound;
        return std::string();
    }

    if (GetTempFileName(tempFolderPath.data(), TEXT("NRF"), 0, tempFilePath.data()) == 0)
    {
        error = TempCouldNotCreateFile;
        return std::string();
    }

    return std::string(tempFilePath.data());
}

void TempFile::deleteFile()
{
    if (pathExists(filename))
    {
        DeleteFile(filename.c_str());
    }

    filename.clear();
}

std::string getHighLevelLibraryName()
{
    return std::string("highlevelnrfjprog.dll");
}

std::string getnrfjprogLibraryName()
{
    return std::string("nrfjprog.dll");
}
