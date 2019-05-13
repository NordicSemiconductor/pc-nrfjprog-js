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

#include "highlevelwrapper.h"
#include "libraryloader.h"
#include "osfiles.h"

LibraryHandleType libraryHandle;

errorcode_t loadHighLevelFunctions(LibraryFunctionPointersType *libraryFunctions)
{
    static std::string highLevelPath{};

    if (highLevelPath.empty())
    {
        std::string libraryName     = getHighLevelLibraryName();
        const errorcode_t finderror = OSFilesFindLibrary(highLevelPath, libraryName);

        if (finderror != errorcode_t::JsSuccess)
        {
            highLevelPath.clear();
            return finderror;
        }
    }

    libraryHandle = LibraryLoad(highLevelPath);

    if (!static_cast<bool>(libraryHandle))
    {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->dll_get_version, "NRFJPROG_dll_version", libraryHandle))
    {
        return errorcode_t::CouldNotLoadDLL;
    }

#define LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(target)                                              \
    do                                                                                             \
    {                                                                                              \
        if (!load_func_ptr(&libraryFunctions->target,                                              \
                           static_cast<const char *>("NRFJPROG_" #target),                         \
                           (libraryHandle)))                                                       \
        {                                                                                          \
            return errorcode_t::CouldNotLoadDLL;                                                   \
        }                                                                                          \
    } while (0);

    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(dll_open);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(dll_close);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(is_dll_open);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(get_connected_probes);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(probe_init);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(DFU_init);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(probe_uninit);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(get_library_info);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(get_probe_info);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(get_device_info);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(program);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_to_file);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(verify);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(erase);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(recover);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(read_u32);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(write);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(write_u32);
    LOAD_FUNCTION_POINTER_RETURN_ON_ERROR(reset);

#undef LOAD_FUNCTION_POINTER_RETURN_ON_ERROR

    return errorcode_t::JsSuccess;
}

errorcode_t releaseHighLevel()
{
    LibraryFree(libraryHandle);
    return errorcode_t::JsSuccess;
}
