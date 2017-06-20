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

#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <dlfcn.h>

#include "../osfiles.h"

#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>

#include <iostream>

std::string OSFilesConcatPaths(std::string base_path, std::string relative_path)
{
    return base_path + '/' + relative_path;
}

bool OSFilesExists(const char * path)
{
    struct stat buffer;
    return ((0 == stat(path, &buffer)));
}

errorcode_t OSFilesFindDll(char * dll_path, int dll_path_len)
{
    char temp_dll_path[dll_path_len];
    memset(temp_dll_path, 0, dll_path_len);

    ssize_t len = readlink("/proc/self/exe", temp_dll_path, dll_path_len - 1);

    if (len == -1)
    {
        return errorcode_t::CouldNotFindJprogDLL;
    }

    strncpy(dll_path, dirname(temp_dll_path), dll_path_len - 1);
    strncat(dll_path, "/libhighlevelnrfjprog.so", dll_path_len - strlen(dll_path) - 1);

    if (!OSFilesExists(dll_path))
    {
        /* It is possible that the user might have place the .so in another folder. In that case dlopen will find it. If it is not found, return JLinkARMDllNotFoundError. */
        void * dll = dlopen("libhighlevelnrfjprog.so", RTLD_LAZY);

        if (dll)
        {
            dlclose(dll);
            strncpy(dll_path, "libhighlevelnrfjprog.so", dll_path_len - 1);
            return errorcode_t::JsSuccess;
        }

        return errorcode_t::CouldNotFindJprogDLL;
    }

    return errorcode_t::JsSuccess;
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
std::string OSFilesGetTempFilePath(void)
{
    std::string temp_file_name_template = OSFilesConcatPaths(OSFilesGetTempFolderPath(), "nRFXXXXXX.hex");

    char temp_file_name[COMMON_MAX_PATH];

    strncpy(temp_file_name, temp_file_name_template.c_str(), COMMON_MAX_PATH);

    int temp_file = mkstemps(temp_file_name, 4);

    if (temp_file == -1)
    {
        return std::string();
    }

    /* mkstemps returns an opened file descriptor. */
    close(temp_file);

    return std::string(temp_file_name);
}


void OSFilesDeleteFile(std::string file_path)
{
    if (OSFilesExists(file_path.c_str()))
    {
        remove(file_path.c_str());
    }
}
