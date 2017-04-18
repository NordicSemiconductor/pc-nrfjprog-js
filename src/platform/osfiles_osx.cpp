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


#include "nrfjprog.h"

#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>

#include <libproc.h>  // proc pidpathinfo maxsize

bool OSFilesExists(char * path)
{
    struct stat buffer;
    return ((0 == stat(path, &buffer)));
}


NrfjprogErrorCodesType OSFilesFindDll(char * dll_path, int dll_path_len)
{
    char temp_dll_path[dll_path_len];
    memset(temp_dll_path, 0, dll_path_len);

    int ret;
    pid_t pid;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];

    pid = getpid();
    ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));
    if (ret <= 0) {
        // PID not found, error
        return NrfjprogDllNotFoundError;
    }

    strncpy(dll_path, dirname(pathbuf), dll_path_len - 1);
    strncat(dll_path, "/libnrfjprogdll.dylib", dll_path_len - strlen(dll_path) - 1);

    if (!OSFilesExists(dll_path)) {
        /* It is possible that the user might have place the .dylib in another folder. In that case dlopen will find it. If it is not found, return JLinkARMDllNotFoundError. */
        void * dll = dlopen("libnrfjprogdll.dylib", RTLD_LAZY);
        if (dll){
            dlclose(dll);
            strncpy(dll_path, "libnrfjprogdll.dylib", dll_path_len - 1);
            return Success;
        }
        return NrfjprogDllNotFoundError;
    }
    return Success;
}


NrfjprogErrorCodesType OSFilesFindJLink(char * jlink_path, int jlink_path_len)
{

    DIR * directory = opendir("/Applications/SEGGER/JLink/");
    if (directory == NULL){

        /* It is possible that the user might have place the .dylib in another folder. In that case dlopen will find it. If it is not found, return JLinkARMDllNotFoundError. */
        void * dll = dlopen("libjlinkarm.dylib", RTLD_LAZY);
        if (dll){
            dlclose(dll);
            strncpy(jlink_path, "libjlinkarm.dylib", jlink_path_len - 1);
            return Success;
        }

        return JLinkARMDllNotFoundError;
    }

    char candidate[jlink_path_len];
    strncpy(candidate, "libjlinkarm.0.0.0.dylib", jlink_path_len - 1);

    struct dirent * directory_entry;
    while (NULL != (directory_entry = readdir(directory))){
        /* Look for either .dylib, or highest .[NUM].dylib */

        /* If there is no ".dylib" in file name, continue looping in the files of the directory. */
        if (strstr(directory_entry->d_name, ".dylib") == NULL){
            continue;
        }

        if (strcmp(directory_entry->d_name, candidate) > 0) {
            strncpy(candidate, directory_entry->d_name, jlink_path_len - 1);
        }
    }
    closedir(directory);

    strncpy(jlink_path, "/Applications/SEGGER/JLink/", jlink_path_len - 1);
    strncat(jlink_path, candidate, jlink_path_len - strlen(jlink_path) - 1);

    if (!OSFilesExists(jlink_path)){
        return JLinkARMDllNotFoundError;
    }

    return Success;
}

NrfjprogErrorCodesType OSFilesFindIni(char * ini_path, int ini_path_len)
{
    char temp_ini_path[ini_path_len];
    memset(temp_ini_path, 0, ini_path_len);

    int ret;
    pid_t pid;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];

    pid = getpid();
    ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));
    if (ret <= 0) {
        return NrfjprogIniNotFoundError;
    }

    strncpy(ini_path, dirname(pathbuf), ini_path_len - 1);
    strncat(ini_path, "/nrfjprog.ini", ini_path_len - strlen(ini_path) - 1);

    if (!OSFilesExists(ini_path)) {
        return NrfjprogIniNotFoundError;
    }
    return Success;
}
