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
#include <vector>

#include "../../osfiles.h"
#include "../../utility/conversion.h"

#include "../../nan_wrap.h"

#include "Shlwapi.h"
#include <windows.h>

#pragma comment(lib, "Shlwapi.lib")

#define MAX_KEY_LENGTH 1000
#define MAX_VALUE_NAME 1000

std::string * pLibrarySearchPath = nullptr;

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

errorcode_t OSFilesFindLibrary(std::string & libraryPath, const std::string & fileName)
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

    return errorcode_t::CouldNotFindJprogDLL;
}

bool AbstractFile::pathExists(const char * path)
{
    const int wchars_num = MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);
    auto wstr            = std::vector<wchar_t>(wchars_num);
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wstr.data(), wchars_num);

    return PathFileExistsW(wstr.data()) == TRUE;
}

std::string TempFile::getTempFileName()
{
    /* Folder name should never be longer than MAX_PATH-14 characters to be compatible with
     * GetTempFileName function. */
    std::vector<char> tempFolderPath(MAX_PATH);
    std::vector<char> tempFilePath(MAX_PATH);

    const DWORD pathLength = GetTempPath(MAX_PATH, tempFolderPath.data());

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

    return std::string(tempFilePath.data()) + ".hex";
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
