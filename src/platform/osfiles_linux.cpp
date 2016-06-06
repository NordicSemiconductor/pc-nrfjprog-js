
#define _CRT_SECURE_NO_WARNINGS

#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <dlfcn.h>


#include "nrfjprog.h"
#include "../osfiles.h"

#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>

#include <iostream>



bool OSFilesExists(char * path)
{
    struct stat buffer;
    return ((0 == stat(path, &buffer)));
}


NrfjprogErrorCodesType OSFilesFindDll(char * dll_path, int dll_path_len)
{
    char temp_dll_path[dll_path_len];
    memset(temp_dll_path, 0, dll_path_len);

    ssize_t len = readlink("/proc/self/exe", temp_dll_path, dll_path_len - 1);
    if (len == -1){
        return NrfjprogDllNotFoundError;
    }

    strncpy(dll_path, dirname(temp_dll_path), dll_path_len - 1);
    strncat(dll_path, "/libnrfjprogdll.so", dll_path_len - strlen(dll_path) - 1);

    if (!OSFilesExists(dll_path)) {
        /* It is possible that the user might have place the .so in another folder. In that case dlopen will find it. If it is not found, return JLinkARMDllNotFoundError. */
        void * dll = dlopen("libnrfjprogdll.so", RTLD_LAZY);
        if (dll){
            dlclose(dll);
            strncpy(dll_path, "libnrfjprogdll.so", dll_path_len - 1);
            return Success;
        }

        return NrfjprogDllNotFoundError;
    }
    return Success;
}

NrfjprogErrorCodesType OSFilesFindJLink(char * jlink_path, int jlink_path_len)
{

    DIR * directory = opendir("/opt/SEGGER/JLink/");
    if (directory == NULL){

        /* It is possible that the user might have place the .so in another folder. In that case dlopen will find it. If it is not found, return JLinkARMDllNotFoundError. */
        void * dll = dlopen("libjlinkarm.so", RTLD_LAZY);
        if (dll){
            dlclose(dll);
            strncpy(jlink_path, "libjlinkarm.so", jlink_path_len - 1);
            return Success;
        }

        return JLinkARMDllNotFoundError;
    }

    char candidate[jlink_path_len];
    char temp_candidate[jlink_path_len];
    char temp_new_possible_candidate[jlink_path_len];
    strncpy(candidate, "libjlinkarm.so.0.0.0", jlink_path_len - 1);
    strncpy(temp_candidate, "libjlinkarm.so.0.0.0.dummy", jlink_path_len - 1);
    strncpy(temp_new_possible_candidate, "libjlinkarm.so.0.0.0.dummy", jlink_path_len - 1);

    struct dirent * directory_entry;
    while (NULL != (directory_entry = readdir(directory))){
        /* Look for either .so, or highest .so.[NUM] */

        /* If there is no "libjlinkarm.so" in file name, continue looping in the files of the directory. */
        if (strstr(directory_entry->d_name, "libjlinkarm.so") == NULL){
            continue;
        }
        if (strstr(directory_entry->d_name, "libjlinkarm.so") != directory_entry->d_name){
            continue;
        }

        strncpy(temp_new_possible_candidate, directory_entry->d_name, jlink_path_len - 1);
        strncat(temp_new_possible_candidate, ".dummy", jlink_path_len - strlen(temp_new_possible_candidate) - 1);

        if (strcmp(temp_new_possible_candidate, temp_candidate) > 0){
            strncpy(candidate, directory_entry->d_name, jlink_path_len - 1);
            strncpy(temp_candidate, temp_new_possible_candidate, jlink_path_len - 1);
        }
    }
    closedir(directory);

    strncpy(jlink_path, "/opt/SEGGER/JLink/", jlink_path_len - 1);
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

    ssize_t len = readlink("/proc/self/exe", temp_ini_path, ini_path_len - 1);
    if (len == -1){
        return NrfjprogIniNotFoundError;
    }

    strncpy(ini_path, dirname(temp_ini_path), ini_path_len - 1);
    strncat(ini_path, "/nrfjprog.ini", ini_path_len - strlen(ini_path) - 1);

    if (!OSFilesExists(ini_path)) {
        return NrfjprogIniNotFoundError;
    }
    return Success;
}
