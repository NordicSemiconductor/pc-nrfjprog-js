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

#include "osfiles.h"

#include <fstream>
#include <iostream>

void OSFilesInit(v8::Local<v8::Object> target)
{
    Nan::SetMethod(target, "setLibrarySearchPath", OSFilesSetLibrarySearchPath);
}

FileFormatHandler::FileFormatHandler(const std::string &fileinfo, input_format_t inputFormat)
{
    if (inputFormat == INPUT_FORMAT_HEX_STRING)
    {
        file = std::unique_ptr<AbstractFile>(new TempFile(fileinfo));
    }
    else
    {
        file = std::unique_ptr<AbstractFile>(new LocalFile(fileinfo));
    }
}

std::string FileFormatHandler::getFileName()
{
    return file->getFileName();
}

bool FileFormatHandler::exists()
{
    return AbstractFile::pathExists(getFileName());
}

std::string FileFormatHandler::errormessage()
{
    return file->errormessage();
}

std::string AbstractFile::getFileName()
{
    return filename;
}

bool AbstractFile::pathExists(const std::string &path)
{
    return pathExists(path.c_str());
}

TempFile::TempFile(const std::string &fileContent)
    : error(TempNoError)
{
    filename = writeTempFile(fileContent);
}

TempFile::~TempFile()
{
    deleteFile();
}

std::string TempFile::writeTempFile(const std::string &fileContent)
{
    std::string filePath = getTempFileName();

    if (filePath.empty())
    {
        return std::string();
    }

    std::ofstream outputfile;
    outputfile.open(filePath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    outputfile << fileContent;
    outputfile.close();

    return filePath;
}

std::string TempFile::errormessage()
{
    switch (error)
    {
        case TempPathNotFound:
            return "Could not find a location to store the temp file";
        case TempCouldNotCreateFile:
            return "Could not create the temporary file";
        case TempNoError:
        default:
            break;
    }
    return "No error";
}

LocalFile::LocalFile(const std::string &fileName)
{
    filename = fileName;
}

std::string LocalFile::errormessage()
{
    return "Could not find the file " + filename;
}
