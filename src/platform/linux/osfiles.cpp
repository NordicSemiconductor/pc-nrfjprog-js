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

#include "../../osfiles.h"
#include "../../utility/conversion.h"

#include <nan.h>

#include <dlfcn.h>
#include <libgen.h>
#include <string>
#include <sys/stat.h>

#include <dirent.h>
#include <sys/types.h>

#include <unistd.h>

#include <iostream>

std::string *pLibrarySearchPath = nullptr;

// NAN_METHOD is a macro; it's shorthand so you don't have to write that the
// first and only parameter is of type Nan::FunctionCallbackInfo<v8::Value>
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

/* Try to locate a dynamically-linked library named fileName, and set libraryPath
 * to the full path to that library.
 */
errorcode_t OSFilesFindLibrary(std::string &libraryPath, const std::string &fileName)
{
    std::vector<char> tempLibraryPath(COMMON_MAX_PATH, '\0');

    // Fetch path of currently running executable
    ssize_t len;
    len = readlink("/proc/self/exe", tempLibraryPath.data(), COMMON_MAX_PATH - 1);

    if (len == -1)
    {
        return errorcode_t::CouldNotFindJprogDLL;
    }

    // If there is a file with the requested fileName in the same path as the
    // current node.js (or electron) executable, use that.
    libraryPath.append(dirname(tempLibraryPath.data()));
    libraryPath.append("/");
    libraryPath.append(fileName);
    if (AbstractFile::pathExists(libraryPath))
    {
        return errorcode_t::JsSuccess;
    }

    // Try the path specified from calling OSFilesSetLibrarySearchPath
    if (pLibrarySearchPath != nullptr)
    {
        libraryPath.assign(*pLibrarySearchPath);
    }
    else
    {
        libraryPath.assign("");
    }
    libraryPath.append("/");
    libraryPath.append(fileName);
    if (AbstractFile::pathExists(libraryPath))
    {
        return errorcode_t::JsSuccess;
    }

    // Last recourse, try loading the library through dlopen().
    // That will look into /usr/lib and into whatever LD_LIBRARY_PATH looks into.
    void *libraryHandle = dlopen(fileName.c_str(), RTLD_LAZY);

    if (libraryHandle != nullptr)
    {
        dlclose(libraryHandle);
        libraryPath = fileName;
        return errorcode_t::JsSuccess;
    }

    // If the library hasn't been found, return JLinkARMDllNotFoundError
    return errorcode_t::CouldNotFindJprogDLL;
}

bool AbstractFile::pathExists(const char *path)
{
    struct stat buffer
    {};
    return ((0 == stat(path, &buffer)));
}

/* Return the temp folder found by checking TMPDIR, TMP, TEMP, or TEMPDIR. If none of these are
 * valid, "/tmp" is returned. */
std::string OSFilesGetTempFolderPath()
{
    static const std::vector<std::string> temp_keys = {"TMPDIR", "TMP", "TEMP", "TEMPDIR"};

    for (const std::string &temp_key : temp_keys)
    {
        char *val = getenv(temp_key.c_str());

        if (val != nullptr)
        {
            return std::string(val);
        }
    }

    return std::string("/tmp");
}

/* Return a valid, unique file name in the temp folder.
 * The temp folder is found by checking TMPDIR, TMP, TEMP, or TEMPDIR. If none of these are found,
 * "/tmp" is used. */
std::string TempFile::getTempFileName()
{
    std::string tempFileNameTemplate = OSFilesGetTempFolderPath() + "/nRFXXXXXX.hex";

    std::vector<char> tempFileName(COMMON_MAX_PATH, '\0');

    strncpy(tempFileName.data(), tempFileNameTemplate.c_str(), COMMON_MAX_PATH);

    int temp_file = mkstemps(tempFileName.data(), 4);

    if (temp_file == -1)
    {
        error = TempCouldNotCreateFile;
        return std::string();
    }

    /* mkstemps returns an opened file descriptor. */
    close(temp_file);

    return std::string(tempFileName.data());
}

void TempFile::deleteFile()
{
    if (pathExists(filename))
    {
        remove(filename.c_str());
    }

    filename.clear();
}

std::string getHighLevelLibraryName()
{
    return std::string("libhighlevelnrfjprog.so");
}

std::string getnrfjprogLibraryName()
{
    return std::string("libnrfjprogdll.so");
}
