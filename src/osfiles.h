#ifndef OS_FILES_H
#define OS_FILES_H

#include "nrfjprog.h"

#define COMMON_MAX_PATH  (4096)   /* Arbitrarily selected MAX_PATH for every platform. */
#define COMMON_MAX_COMMAND_LINE  (8191) /* Arbitrarily selected MAX_COMMAND_LINE_LENGTH for every platform, according to limit for windows: http://stackoverflow.com/questions/3205027/maximum-length-of-command-line-string. */
#define COMMON_MAX_INI_LINE (1024)

NrfjprogErrorCodesType OSFilesFindDll(char * dll_path, int dll_path_len);
NrfjprogErrorCodesType OSFilesFindJLink(char * jlink_path, int jlink_path_len);
NrfjprogErrorCodesType OSFilesFindIni(char * ini_path, int ini_path_len);

bool OSFilesExists(char * path);

#endif
