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
std::string highLevelPath;

errorcode_t loadHighLevelFunctions(LibraryFunctionPointersType * libraryFunctions)
{
    if (highLevelPath.empty()) {
        std::string libraryName = getHighLevelLibraryName();
        const errorcode_t finderror = OSFilesFindLibrary(highLevelPath, libraryName);

        if (finderror != errorcode_t::JsSuccess) {
            return finderror;
        }
    }

    libraryHandle = LibraryLoad(highLevelPath);

    if (!libraryHandle){
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->dll_get_version, "NRFJPROG_dll_version", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->dll_open, "NRFJPROG_dll_open", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->dll_close, "NRFJPROG_dll_close", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->is_dll_open, "NRFJPROG_is_dll_open", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->get_connected_probes, "NRFJPROG_get_connected_probes", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->probe_init, "NRFJPROG_probe_init", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->probe_uninit, "NRFJPROG_probe_uninit", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->get_library_info, "NRFJPROG_get_library_info", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->get_probe_info, "NRFJPROG_get_probe_info", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->get_device_info, "NRFJPROG_get_device_info", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->program, "NRFJPROG_program", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->read_to_file, "NRFJPROG_read_to_file", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->verify, "NRFJPROG_verify", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->erase, "NRFJPROG_erase", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->recover, "NRFJPROG_recover", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->read, "NRFJPROG_read", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->read_u32, "NRFJPROG_read_u32", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->write, "NRFJPROG_write", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->write_u32, "NRFJPROG_write_u32", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->reset, "NRFJPROG_reset", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    if (!load_func_ptr(&libraryFunctions->run, "NRFJPROG_run", libraryHandle)) {
        return errorcode_t::CouldNotLoadDLL;
    }

    return errorcode_t::JsSuccess;
}

errorcode_t releaseHighLevel() {
    LibraryFree(libraryHandle);
    return errorcode_t::JsSuccess;
}
