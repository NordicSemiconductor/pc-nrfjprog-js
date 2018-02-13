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

#include "../../osfiles.h"

#include <nan.h>

#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>

#include <libproc.h>  // proc pidpathinfo maxsize

std::string librarySearchPath;

NAN_METHOD(OSFilesSetLibrarySearchPath)
{
    // Parse parameter from the FunctionCallbackInfo received, convert into a std::string
    if (info.Length() > 0) {
        if (info[0]->IsString()) {
            v8::String::Utf8Value param1(info[0]->ToString());
            std::string path = std::string(*param1);

            printf("\nosx/osfiles.cpp: setLibrarySearchPath() called with %s\n\n",
                path.c_str()
            );

            librarySearchPath.assign(path);
        }
    }
    /// TODO: Add some error throwing if no parameters or the parameter is not a string
}


errorcode_t OSFilesFindLibrary(std::string &libraryPath, std::string &fileName)
{
    int ret;
    pid_t pid;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];

    // Fetch path of currently running executable
    pid = getpid();
    ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));

    if (ret <= 0)
    {
        // PID not found, error
        return errorcode_t::CouldNotFindJprogDLL;
    }

    // If there is a file with the requested fileName in the same path as the
    // current node.js (or electron) executable, use that.
    libraryPath.append(dirname(pathbuf));
    libraryPath.append("/");
    libraryPath.append(fileName);
    if (AbstractFile::pathExists(libraryPath))
    {
        return errorcode_t::JsSuccess;
    }

    // Try the path specified from calling OSFilesSetLibrarySearchPath
    libraryPath.assign(librarySearchPath);
    libraryPath.append("/");
    libraryPath.append(fileName);
    if (AbstractFile::pathExists(libraryPath))
    {
        printf("\nShared jprog libraries found in specified path: %s \n\n", libraryPath.c_str());
        return errorcode_t::JsSuccess;
    } else {
        printf("\nShared jprog libraries **NOT** found in specified path :-( %s \n\n", libraryPath.c_str());
    }

    // Last recourse, try loading the library through dlopen().
    // That will look into /usr/lib and into whatever LD_LIBRARY_PATH looks into.
    void * libraryHandle = dlopen(fileName.c_str(), RTLD_LAZY);

    if (libraryHandle)
    {
        dlclose(libraryHandle);
        libraryPath = fileName;
        return errorcode_t::JsSuccess;
    }

    return errorcode_t::CouldNotFindJprogDLL;

}

std::string TempFile::concatPaths(std::string base_path, std::string relative_path)
{
    return base_path + '/' + relative_path;
}

bool AbstractFile::pathExists(const char * path)
{
    struct stat buffer;
    return ((0 == stat(path, &buffer)));
}

/* Return the temp folder found by checking TMPDIR, TMP, TEMP, or TEMPDIR. If none of these are valid, "/tmp" is returned. */
std::string OSFilesGetTempFolderPath(void)
{
    std::string temp_keys[4] = {
        "TMPDIR",
        "TMP",
        "TEMP",
        "TEMPDIR"
    };

    for (uint32_t i = 0; i < 4; i++)
    {
        char * val = getenv(temp_keys[i].c_str());

        if (val != NULL)
        {
            return std::string(val);
        }
    }

    return std::string("/tmp");
}

/* Return a valid, unique file name in the temp folder.
 * The temp folder is found by checking TMPDIR, TMP, TEMP, or TEMPDIR. If none of these are found, "/tmp" is used. */
std::string TempFile::getTempFileName()
{
    std::string temp_file_name_template = concatPaths(OSFilesGetTempFolderPath(), "nRFXXXXXX.hex");

    char temp_file_name[COMMON_MAX_PATH];

    strncpy(temp_file_name, temp_file_name_template.c_str(), COMMON_MAX_PATH);

    int temp_file = mkstemps(temp_file_name, 4);

    if (temp_file == -1)
    {
        error = TempCouldNotCreateFile;
        return std::string();
    }

    /* mkstemps returns an opened file descriptor. */
    close(temp_file);

    return std::string(temp_file_name);
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
    return std::string("libhighlevelnrfjprog.dylib");
}

std::string getnrfjprogLibraryName()
{
    return std::string("libnrfjprogdll.dylib");
}
