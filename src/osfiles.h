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

#ifndef OS_FILES_H
#define OS_FILES_H

#include "highlevel_common.h"

#include <memory>

#include <nan.h>

#define COMMON_MAX_PATH  (4096)   /* Arbitrarily selected MAX_PATH for every platform. */
#define COMMON_MAX_COMMAND_LINE  (8191) /* Arbitrarily selected MAX_COMMAND_LINE_LENGTH for every platform, according to limit for windows: http://stackoverflow.com/questions/3205027/maximum-length-of-command-line-string. */
#define COMMON_MAX_INI_LINE (1024)

void OSFilesInit(v8::Local<v8::Object> tpl);

NAN_METHOD(OSFilesSetLibrarySearchPath);
errorcode_t OSFilesFindLibrary(std::string &libraryPath, std::string &fileName);
std::string getHighLevelLibraryName();
std::string getnrfjprogLibraryName();

class AbstractFile
{
public:
    virtual ~AbstractFile() {};

    std::string getFileName();
    virtual std::string errormessage() = 0;

    static bool pathExists(const std::string &path);
    static bool pathExists(const char *path);

protected:
    std::string filename;
};

class LocalFile : public AbstractFile
{
public:
    LocalFile(std::string filename);
    virtual std::string errormessage() override;
};

class TempFile : public AbstractFile
{
public:
    TempFile(std::string fileContent);
    virtual ~TempFile() override;
    virtual std::string errormessage() override;

private:
    std::string writeTempFile(std::string fileContent);
    std::string getTempFileName();
    void deleteFile();
    std::string concatPaths(std::string base_path, std::string relative_path);

    enum TempFileErrorcode {
        TempNoError,
        TempPathNotFound,
        TempCouldNotCreateFile
    } error;
};

class FileFormatHandler
{
public:
    FileFormatHandler(std::string fileinfo, input_format_t inputFormat);

    std::string getFileName();
    bool exists();
    std::string errormessage();

private:
    std::unique_ptr<AbstractFile> file;
};

#endif
