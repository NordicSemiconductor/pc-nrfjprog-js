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

#include "highlevel.h"

#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <vector>

#include "highlevel_batons.h"
#include "highlevel_common.h"
#include "highlevel_helpers.h"

#include "utility/conversion.h"
#include "utility/errormessage.h"
#include "utility/utility.h"

constexpr int MAX_SERIAL_NUMBERS = 100;

struct HighLevelStaticPrivate
{
    bool loaded{false};
    std::string logMessage;
    std::timed_mutex logMutex;
    std::unique_ptr<Nan::Callback> jsProgressCallback;
    std::unique_ptr<uv_async_t> progressEvent;
    std::mutex progressProcessMutex;
    std::queue<std::string> progressProcess;
    std::chrono::high_resolution_clock::time_point rttStartTime{};

    std::map<uint32_t, Probe_handle_t> openProbeMap{};
    std::mutex openProbeMapMutex;

    static inline Nan::Persistent<v8::Function> & constructor()
    {
        static Nan::Persistent<v8::Function> my_constructor;
        return my_constructor;
    }

    nrfjprogdll_err_t registerProbe(const uint32_t serialNumber, Probe_handle_t probe)
    {
        std::unique_lock<std::mutex> lock(openProbeMapMutex);
        if (openProbeMap.find(serialNumber) != openProbeMap.end())
        {
            return INVALID_OPERATION; // Already opened
        }

        openProbeMap[serialNumber] = probe;
        return SUCCESS;
    }

    void unregisterProbe(const uint32_t serialNumber)
    {
        std::unique_lock<std::mutex> lock(openProbeMapMutex);
        openProbeMap.erase(serialNumber);
    }

    void unregisterProbe(const Probe_handle_t probe)
    {
        std::unique_lock<std::mutex> lock(openProbeMapMutex);
        const auto it = std::find_if(
            openProbeMap.begin(),
            openProbeMap.end(),
            [probe](std::pair<const uint32_t, Probe_handle_t> v) { return v.second == probe; }
        );
        if (it != openProbeMap.end())
        {
            openProbeMap.erase(it);
        }
    }

    Probe_handle_t getProbe(const uint32_t serialNumber)
    {
        std::unique_lock<std::mutex> lock(openProbeMapMutex);
        const auto it = openProbeMap.find(serialNumber);
        if (it != openProbeMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    inline bool hasProbe(const uint32_t serialNumber)
    {
        return getProbe(serialNumber) != nullptr;
    }

    const std::vector<coprocessor_t> coProcessors{ CP_APPLICATION, CP_NETWORK };
};

static HighLevelStaticPrivate * pHighlvlStatic = nullptr;

std::timed_mutex Baton::executionMutex;

static void closeLibrary(void *)
{
    NRFJPROG_dll_close();
}

NAN_MODULE_INIT(HighLevel::Init)
{
    node::AddEnvironmentCleanupHook(v8::Isolate::GetCurrent(), closeLibrary, nullptr);

    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("nRFjprog").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    init(tpl);

    HighLevelStaticPrivate::constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("nRFjprog").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(HighLevel::New)
{
    if (info.IsConstructCall())
    {
        (new HighLevel())->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        const int argc                     = 1;
        v8::Local<v8::Value> argv[argc]    = {info[0]};
        const v8::Local<v8::Function> cons = Nan::New(HighLevelStaticPrivate::constructor());
        info.GetReturnValue().Set(
            Nan::NewInstance(cons, argc, static_cast<v8::Local<v8::Value> *>(argv)).ToLocalChecked());
    }
}

HighLevel::HighLevel()
{
    static HighLevelStaticPrivate highLevelStaticPrivate;
    pHighlvlStatic = &highLevelStaticPrivate;
    resetLog();
    NRFJPROG_dll_open(nullptr, &HighLevel::log);
}

void HighLevel::CallFunction(Nan::NAN_METHOD_ARGS_TYPE info,
                             const parse_parameters_function_t & parse,
                             const execute_function_t & execute,
                             const return_function_t & ret,
                             const bool hasSerialNumber)
{
    // This is a check that there exists a parse- and execute function, both of which are
    // needed to parse arguments and execute the function.
    // If this shows up in production, it is due to missing functions in the relevant
    // NAN_METHOD defining the functions.
    if (parse == nullptr || execute == nullptr)
    {
        auto message = ErrorMessage::getErrorMessage(
            errorcode_t::CouldNotCallFunction, nrfjprog_js_err_map,
            std::string("One or more of the parse, or execute functions is missing for this function"));
        Nan::ThrowError(message);
        return;
    }

    resetLog();

    auto argumentCount = 0;
    std::unique_ptr<Baton> baton;
    uint32_t serialNumber = 0;
    coprocessor_t coProcessor = CP_APPLICATION;

    pHighlvlStatic->jsProgressCallback.reset();

    try
    {
        if (hasSerialNumber)
        {
            try
            {
                serialNumber = Convert::getNativeUint32(info[argumentCount]);
                ++argumentCount;
            }
            catch (...)
            {
                const auto str = Convert::getNativeString(info[argumentCount]);
                ++argumentCount;

                char sep;
                uint32_t cop;
                if (3 != std::sscanf(str.c_str(), "%u%c%u", &serialNumber, &sep, &cop)) {
                    auto message = ErrorMessage::getErrorMessage(
                        errorcode_t::CouldNotCallFunction, nrfjprog_js_err_map,
                        std::string("parsing serialnumber:coprocessor"));
                    Nan::ThrowError(message);
                    return;
                }
                if (cop >= pHighlvlStatic->coProcessors.size()) {
                    auto message = ErrorMessage::getErrorMessage(
                        errorcode_t::CouldNotCallFunction, nrfjprog_js_err_map,
                        std::string("parsing coprocessor number"));
                    Nan::ThrowError(message);
                    return;
                }
                coProcessor = pHighlvlStatic->coProcessors[cop];
            }
        }

        baton.reset(parse(info, argumentCount));

        if (baton->mayHaveProgressCallback && (argumentCount + 1) < info.Length())
        {
            v8::Local<v8::Function> callback   = Convert::getCallbackFunction(info[argumentCount]);
            pHighlvlStatic->jsProgressCallback = std::make_unique<Nan::Callback>(callback);
            argumentCount++;
        }

        v8::Local<v8::Function> callback = Convert::getCallbackFunction(info[argumentCount]);
        baton->callback                  = std::make_unique<Nan::Callback>(callback);
        argumentCount++;

        if (info.Length() > argumentCount)
        {
            argumentCount = CUSTOM_ARGUMENT_PARSE_ERROR;
            std::ostringstream errorStringStream;
            errorStringStream << "Too many parameters. The function " << baton->name << " does not take "
                              << info.Length() << " parameters.";
            throw std::runtime_error(errorStringStream.str());
        }
    }
    catch (const std::runtime_error & error)
    {
        baton = nullptr;

        pHighlvlStatic->jsProgressCallback.reset();

        const auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error.what());
        Nan::ThrowTypeError(message);

        return;
    }

    // This is a check that there exists a returnfunction when there are more returns
    // than just err. If this shows up in production, it is due to missing return function
    if (ret == nullptr && baton->returnParameterCount > 0)
    {
        const auto message = ErrorMessage::getErrorMessage(
            1,
            nrfjprog_js_err_map,
            std::string("The return function has more than one parameter and is required for this "
                        "function, but is missing"));
        Nan::ThrowError(message);
        return;
    }

    baton->executeFunction = execute;
    baton->returnFunction  = ret;
    baton->serialNumber    = serialNumber;
    baton->coProcessor     = coProcessor;

    uv_queue_work(
        uv_default_loop(), baton->req.get(), ExecuteFunction, reinterpret_cast<uv_after_work_cb>(ReturnFunction));

    (void)baton.release();
}

void HighLevel::ExecuteFunction(uv_work_t * req)
{
    auto baton = static_cast<Baton *>(req->data);

    std::unique_lock<std::timed_mutex> lock(Baton::executionMutex, std::defer_lock);

    if (!lock.try_lock_for(std::chrono::seconds(10)))
    {
        baton->result = CouldNotExecuteDueToLoad;
        return;
    }

    if (pHighlvlStatic->jsProgressCallback)
    {
        pHighlvlStatic->progressEvent = std::make_unique<uv_async_t>();
        uv_async_init(uv_default_loop(), pHighlvlStatic->progressEvent.get(), sendProgress);
    }

    if (baton->serialNumber != 0)
    {
        nrfjprogdll_err_t initError = SUCCESS;

        auto probe = pHighlvlStatic->getProbe(baton->serialNumber);
        if (probe)
        {
            baton->probe = probe;
        }
        else if (baton->probeType == DFU_PROBE)
        {
            // TODO: do not store in pHighlvlStatic
            initError = NRFJPROG_dfu_init(&(baton->probe),
                                          &HighLevel::progressCallback,
                                          &HighLevel::log,
                                          baton->serialNumber,
                                          CP_MODEM,
                                          nullptr);
        }
        else if (baton->probeType == MCUBOOT_PROBE)
        {
            const auto baton2 = dynamic_cast<ProgramMcuBootDFUBaton *>(baton);

            initError = NRFJPROG_mcuboot_dfu_init(&(baton->probe),
                                                  &HighLevel::progressCallback,
                                                  &HighLevel::log,
                                                  baton2->uart.c_str(),
                                                  baton2->baudRate,
                                                  baton2->responseTimeout);
        }
        else if (baton->probeType == MODEMUARTDFU_PROBE)
        {
            const auto baton2 = dynamic_cast<ProgramModemUartDFUBaton *>(baton);

            initError = NRFJPROG_modemdfu_dfu_serial_init(&(baton->probe),
                                                          &HighLevel::progressCallback,
                                                          &HighLevel::log,
                                                          baton2->uart.c_str(),
                                                          baton2->baudRate,
                                                          baton2->responseTimeout);
        }
        else
        {
            initError = NRFJPROG_probe_init(
                &(baton->probe), &HighLevel::progressCallback, &HighLevel::log, baton->serialNumber, nullptr);

            if (initError == SUCCESS && baton->coProcessor != CP_APPLICATION)
            {
                initError = NRFJPROG_probe_set_coprocessor(baton->probe, baton->coProcessor);
            }
        }

        if (initError != SUCCESS)
        {
            baton->result        = errorcode_t::CouldNotOpenDevice;
            baton->lowlevelError = initError;
            return;
        }
    }

    const auto executeError = baton->executeFunction(baton);

    if (pHighlvlStatic->getProbe(baton->serialNumber) == nullptr)
    {
        if (baton->serialNumber != 0)
        {
            if (baton->probeType == DEBUG_PROBE && baton->cpuNeedsReset)
            {
                const auto resetError = NRFJPROG_reset(baton->probe, RESET_SYSTEM);

                if (resetError != SUCCESS)
                {
                    baton->result        = errorcode_t::CouldNotResetDevice;
                    baton->lowlevelError = resetError;
                    return;
                }
            }

            const auto uninitError = NRFJPROG_probe_uninit(&(baton->probe));

            if (uninitError != SUCCESS)
            {
                baton->result        = errorcode_t::CouldNotCloseDevice;
                baton->lowlevelError = uninitError;
                return;
            }
        }
    }

    if (executeError != SUCCESS)
    {
        baton->result        = errorcode_t::CouldNotCallFunction;
        baton->lowlevelError = executeError;
    }

    if (pHighlvlStatic->progressEvent)
    {
        const auto handle = reinterpret_cast<uv_handle_t *>(pHighlvlStatic->progressEvent.get());

        uv_close(handle, [](uv_handle_t * /*closeHandle*/) { pHighlvlStatic->progressEvent = nullptr; });
    }
}

void HighLevel::ReturnFunction(uv_work_t * req)
{
    Nan::HandleScope scope;

    std::unique_ptr<Baton> baton(static_cast<Baton *>(req->data));
    std::vector<v8::Local<v8::Value>> argv;

    std::string msg;

    {
        std::unique_lock<std::timed_mutex> lock(pHighlvlStatic->logMutex, std::defer_lock);

        if (lock.try_lock_for(std::chrono::seconds(10)))
        {
            msg = pHighlvlStatic->logMessage;
        }
    }

    argv.emplace_back(
        ErrorMessage::getErrorMessage(baton->result, nrfjprog_js_err_map, baton->name, msg, baton->lowlevelError));

    if (baton->result != errorcode_t::JsSuccess)
    {
        for (auto i = 0; i < baton->returnParameterCount; i++)
        {
            argv.emplace_back(Nan::Undefined());
        }
    }
    else
    {
        if (baton->returnFunction != nullptr)
        {
            std::vector<v8::Local<v8::Value>> vector = baton->returnFunction(baton.get());

            argv.insert(argv.end(), vector.begin(), vector.end());
        }
    }

    pHighlvlStatic->jsProgressCallback.reset();

    Nan::AsyncResource resource("pc-nrfjprog-js:callback");
    baton->callback->Call(baton->returnParameterCount + 1, argv.data(), &resource);
}

void HighLevel::log(const char * msg)
{
    log(std::string(msg));
}

void HighLevel::log(const std::string & msg)
{
    std::unique_lock<std::timed_mutex> lock(pHighlvlStatic->logMutex, std::defer_lock);

    if (!lock.try_lock_for(std::chrono::seconds(10)))
    {
        return;
    }

    pHighlvlStatic->logMessage.append(msg + '\n');
}

void HighLevel::resetLog()
{
    std::unique_lock<std::timed_mutex> lock(pHighlvlStatic->logMutex, std::defer_lock);

    if (!lock.try_lock_for(std::chrono::seconds(10)))
    {
        return;
    }

    pHighlvlStatic->logMessage.clear();
}

void HighLevel::progressCallback(const char * process)
{
    if (pHighlvlStatic->jsProgressCallback)
    {
        {
            std::unique_lock<std::mutex> lock(pHighlvlStatic->progressProcessMutex);
            pHighlvlStatic->progressProcess.emplace(process);
        }

        uv_async_send(pHighlvlStatic->progressEvent.get());
    }
}

void HighLevel::sendProgress(uv_async_t * /*handle*/)
{
    if (!pHighlvlStatic->jsProgressCallback)
    {
        return;
    }

    Nan::HandleScope scope;
    std::unique_lock<std::mutex> lock(pHighlvlStatic->progressProcessMutex);

    while (!pHighlvlStatic->progressProcess.empty())
    {
        std::string process = pHighlvlStatic->progressProcess.front();
        pHighlvlStatic->progressProcess.pop();

        v8::Local<v8::Value> argv[1];

        v8::Local<v8::Object> progressObj = Nan::New<v8::Object>();
        Utility::Set(progressObj, "process", Convert::toJsString(process));

        argv[0] = progressObj;

        Nan::AsyncResource resource("pc-nrfjprog-js:callback");
        pHighlvlStatic->jsProgressCallback->Call(1, static_cast<v8::Local<v8::Value> *>(argv), &resource);
    }
}

void HighLevel::init(v8::Local<v8::FunctionTemplate> target)
{
    Nan::SetPrototypeMethod(target, "getDllVersion", GetLibraryVersion); // Deprecated
    Nan::SetPrototypeMethod(target, "getLibraryVersion", GetLibraryVersion);
    Nan::SetPrototypeMethod(target, "getConnectedDevices", GetConnectedDevices);
    Nan::SetPrototypeMethod(target, "getSerialNumbers", GetSerialNumbers);
    Nan::SetPrototypeMethod(target, "getDeviceInfo", GetDeviceInfo);
    Nan::SetPrototypeMethod(target, "getReadbackProtection", GetReadbackProtection);
    Nan::SetPrototypeMethod(target, "getProbeInfo", GetProbeInfo);
    Nan::SetPrototypeMethod(target, "getLibraryInfo", GetLibraryInfo);
    Nan::SetPrototypeMethod(target, "read", Read);
    Nan::SetPrototypeMethod(target, "readU32", ReadU32);

    Nan::SetPrototypeMethod(target, "program", Program);
    Nan::SetPrototypeMethod(target, "programDFU", ProgramDFU);
    Nan::SetPrototypeMethod(target, "programMcuBootDFU", ProgramMcuBootDFU);
    Nan::SetPrototypeMethod(target, "programModemUartDFU", ProgramModemUartDFU);
    Nan::SetPrototypeMethod(target, "readToFile", ReadToFile);
    Nan::SetPrototypeMethod(target, "verify", Verify);
    Nan::SetPrototypeMethod(target, "erase", Erase);

    Nan::SetPrototypeMethod(target, "recover", Recover);
    Nan::SetPrototypeMethod(target, "reset", Reset);

    Nan::SetPrototypeMethod(target, "write", Write);
    Nan::SetPrototypeMethod(target, "writeU32", WriteU32);

    Nan::SetPrototypeMethod(target, "rttStart", RttStart);
    Nan::SetPrototypeMethod(target, "rttStop", RttStop);
    Nan::SetPrototypeMethod(target, "rttRead", RttRead);
    Nan::SetPrototypeMethod(target, "rttWrite", RttWrite);

    Nan::SetPrototypeMethod(target, "open", OpenDevice);
    Nan::SetPrototypeMethod(target, "close", CloseDevice);
}

void HighLevel::initConsts(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
    // nRF51 versions
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAA_REV1); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAA_REV2); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAA_REV3); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAB_REV3); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF51xxx_xxAC_REV3); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF51802_xxAA_REV3); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF51801_xxAB_REV3); // NOLINT(hicpp-signed-bitwise)

    // nRF52805 versions
    NODE_DEFINE_CONSTANT(target, NRF52805_xxAA_REV1);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52805_xxAA_FUTURE); // NOLINT(hicpp-signed-bitwise)

    // nRF52810 versions
    NODE_DEFINE_CONSTANT(target, NRF52810_xxAA_REV1);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52810_xxAA_REV2);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52810_xxAA_FUTURE); // NOLINT(hicpp-signed-bitwise)

    // nRF52811 versions
    NODE_DEFINE_CONSTANT(target, NRF52811_xxAA_REV1);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52811_xxAA_FUTURE); // NOLINT(hicpp-signed-bitwise)

    // nRF52820 versions
    NODE_DEFINE_CONSTANT(target, NRF52820_xxAA_REV1);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52820_xxAA_REV2);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52820_xxAA_FUTURE); // NOLINT(hicpp-signed-bitwise)

    // nRF52832 versions
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_ENGA);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_ENGB);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_REV1);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_REV2);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_REV3);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAA_FUTURE); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_REV1);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_REV2);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_REV3);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52832_xxAB_FUTURE); // NOLINT(hicpp-signed-bitwise)

    // nRF52833 versions
    NODE_DEFINE_CONSTANT(target, NRF52833_xxAA_REV1);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52833_xxAA_FUTURE); // NOLINT(hicpp-signed-bitwise)

    // nRF52840 versions
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_ENGA);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_ENGB);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_REV1);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_REV2);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_REV3);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52840_xxAA_FUTURE); // NOLINT(hicpp-signed-bitwise)

    // nRF53XXX versions
    NODE_DEFINE_CONSTANT(target, NRF5340_xxAA_ENGA);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF5340_xxAA_ENGB);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF5340_xxAA_FUTURE); // NOLINT(hicpp-signed-bitwise)

    // nRF9160 versions
    NODE_DEFINE_CONSTANT(target, NRF9160_xxAA_REV1);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF9160_xxAA_REV2);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF9160_xxAA_FUTURE); // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, NRF51_FAMILY);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF52_FAMILY);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF53_FAMILY);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, NRF91_FAMILY);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, UNKNOWN_FAMILY); // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, ERASE_NONE);                 // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, ERASE_ALL);                  // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, ERASE_PAGES);                // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, ERASE_PAGES_INCLUDING_UICR); // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, JsSuccess);               // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotFindJlinkDLL);    // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotFindJprogDLL);    // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotLoadDLL);         // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotOpenDevice);      // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotOpenDLL);         // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotConnectToDevice); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotCallFunction);    // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotErase);           // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotProgram);         // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotRead);            // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, CouldNotOpenHexFile);     // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, RESET_NONE);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RESET_SYSTEM); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RESET_DEBUG);  // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, RESET_PIN);    // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, ERASE_NONE);                 // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, ERASE_ALL);                  // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, ERASE_PAGES);                // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, ERASE_PAGES_INCLUDING_UICR); // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, VERIFY_NONE); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, VERIFY_READ); // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, INPUT_FORMAT_HEX_FILE);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, INPUT_FORMAT_HEX_STRING); // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, UP_DIRECTION);   // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, DOWN_DIRECTION); // NOLINT(hicpp-signed-bitwise)

    NODE_DEFINE_CONSTANT(target, NONE);     // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, REGION_0); // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, ALL);      // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, BOTH);     // NOLINT(hicpp-signed-bitwise)
    NODE_DEFINE_CONSTANT(target, SECURE);   // NOLINT(hicpp-signed-bitwise)
}

nrfjprogdll_err_t HighLevel::rttCleanup(Probe_handle_t probe)
{
    nrfjprogdll_err_t status = SUCCESS;
    bool started;

    NRFJPROG_is_rtt_started(probe, &started);

    if (started)
    {
        status = NRFJPROG_rtt_stop(probe);
    }

    pHighlvlStatic->unregisterProbe(probe);
    return status;
}

nrfjprogdll_err_t HighLevel::waitForControlBlock(const Probe_handle_t probe, bool & isControlBlockFound)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto controlBlockFound = false;

        const auto status = NRFJPROG_rtt_is_control_block_found(probe, &controlBlockFound);

        if (status != SUCCESS)
        {
            isControlBlockFound = false;
            rttCleanup(probe);
            return status;
        }

        if (controlBlockFound)
        {
            isControlBlockFound = true;
            return SUCCESS;
        }

        auto attemptedStartupTime = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(attemptedStartupTime - pHighlvlStatic->rttStartTime)
                .count() > 5)
        {
            isControlBlockFound = false;
            rttCleanup(probe);
            return SUCCESS;
        }
    }
}

nrfjprogdll_err_t HighLevel::getChannelInformation(RTTStartBaton * baton, bool & isChannelInformationAvailable)
{
    uint32_t downChannelNumber;
    uint32_t upChannelNumber;

    const auto countStatus = NRFJPROG_rtt_read_channel_count(baton->probe, &downChannelNumber, &upChannelNumber);

    if (countStatus != SUCCESS)
    {
        isChannelInformationAvailable = false;
        rttCleanup(baton->probe);
        return countStatus;
    }

    for (uint32_t i = 0; i < downChannelNumber; ++i)
    {
        char channelName[32];
        auto * pChannelName = static_cast<char *>(channelName);
        uint32_t channelSize;

        const auto status =
            NRFJPROG_rtt_read_channel_info(baton->probe, i, DOWN_DIRECTION, pChannelName, 32, &channelSize);
        if (status != SUCCESS)
        {
            isChannelInformationAvailable = false;
            rttCleanup(baton->probe);
            return status;
        }

        std::string name(pChannelName);
        baton->downChannelInfo.emplace_back(std::make_unique<ChannelInfo>(i, DOWN_DIRECTION, name, channelSize));
    }

    for (uint32_t i = 0; i < upChannelNumber; ++i)
    {
        char channelName[32];
        auto * pChannelName = static_cast<char *>(channelName);
        uint32_t channelSize;

        const auto status =
            NRFJPROG_rtt_read_channel_info(baton->probe, i, UP_DIRECTION, pChannelName, 32, &channelSize);

        if (status != SUCCESS)
        {
            isChannelInformationAvailable = false;
            rttCleanup(baton->probe);
            return status;
        }

        std::string name(pChannelName);
        baton->upChannelInfo.emplace_back(std::make_unique<ChannelInfo>(i, UP_DIRECTION, name, channelSize));
    }

    isChannelInformationAvailable = true;

    return SUCCESS;
}

bool HighLevel::isRttStarted(Probe_handle_t probe)
{
    bool started;
    NRFJPROG_is_rtt_started(probe, &started);
    return started;
}

NAN_METHOD(HighLevel::GetLibraryVersion)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new GetLibraryVersionBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<GetLibraryVersionBaton *>(b);
        return NRFJPROG_dll_version(&baton->major, &baton->minor, &baton->revision);
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        const auto baton = dynamic_cast<GetLibraryVersionBaton *>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Object> versionObj = Nan::New<v8::Object>();
        Utility::Set(versionObj, "major", Convert::toJsNumber(baton->major));
        Utility::Set(versionObj, "minor", Convert::toJsNumber(baton->minor));
        Utility::Set(versionObj, "revision", Convert::toJsNumber(baton->revision));

        returnData.emplace_back(versionObj);

        return returnData;
    };

    CallFunction(info, p, e, r, false);
}

NAN_METHOD(HighLevel::GetConnectedDevices)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new GetConnectedDevicesBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<GetConnectedDevicesBaton *>(b);
        uint32_t serialNumbers[MAX_SERIAL_NUMBERS];
        uint32_t available = 0;
        const nrfjprogdll_err_t error =
            NRFJPROG_get_connected_probes(static_cast<uint32_t *>(serialNumbers), MAX_SERIAL_NUMBERS, &available);

        if (error != SUCCESS)
        {
            return error;
        }

        for (uint32_t i = 0; i < available; i++)
        {
            Probe_handle_t getInfoProbe;
            const nrfjprogdll_err_t initError = NRFJPROG_probe_init(
                &getInfoProbe, &HighLevel::progressCallback, &HighLevel::log, serialNumbers[i], nullptr);

            device_info_t device_info;
            probe_info_t probe_info;
            library_info_t library_info;

            if (initError == SUCCESS)
            {
                NRFJPROG_get_device_info(getInfoProbe, &device_info);
                NRFJPROG_get_probe_info(getInfoProbe, &probe_info);
                NRFJPROG_get_library_info(getInfoProbe, &library_info);

                NRFJPROG_probe_uninit(&getInfoProbe);
            }
            baton->probes.emplace_back(
                std::make_unique<ProbeDetails>(serialNumbers[i], device_info, probe_info, library_info));
        }

        return SUCCESS;
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<GetConnectedDevicesBaton *>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Array> connectedDevices = Nan::New<v8::Array>();
        int i                                 = 0;
        for (auto & element : baton->probes)
        {
            Nan::Set(connectedDevices, Convert::toJsNumber(i), element->ToJs());
            i++;
        }

        returnData.emplace_back(connectedDevices);

        return returnData;
    };

    CallFunction(info, p, e, r, false);
}

NAN_METHOD(HighLevel::GetSerialNumbers)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new GetSerialNumbersBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<GetSerialNumbersBaton *>(b);
        uint32_t serialNumbers[MAX_SERIAL_NUMBERS];
        uint32_t available = 0;
        const nrfjprogdll_err_t error =
            NRFJPROG_get_connected_probes(static_cast<uint32_t *>(serialNumbers), MAX_SERIAL_NUMBERS, &available);

        if (error != SUCCESS)
        {
            return error;
        }

        for (uint32_t i = 0; i < available; ++i)
        {
            baton->serialNumbers.emplace_back(serialNumbers[i]);
        }

        return SUCCESS;
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<GetSerialNumbersBaton *>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Array> serialNumbers = Nan::New<v8::Array>();
        int i                              = 0;
        for (auto serialNumber : baton->serialNumbers)
        {
            Nan::Set(serialNumbers, Convert::toJsNumber(i), Convert::toJsNumber(serialNumber));
            ++i;
        }

        returnData.emplace_back(serialNumbers);

        return returnData;
    };

    CallFunction(info, p, e, r, false);
}

NAN_METHOD(HighLevel::GetProbeInfo)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new GetProbeInfoBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<GetProbeInfoBaton *>(b);
        return NRFJPROG_get_probe_info(b->probe, &baton->probeInfo);
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        const auto baton = dynamic_cast<GetProbeInfoBaton *>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(ProbeInfo(baton->probeInfo).ToJs());

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::GetLibraryInfo)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new GetLibraryInfoBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<GetLibraryInfoBaton *>(b);
        return NRFJPROG_get_library_info(b->probe, &baton->libraryInfo);
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        const auto baton = dynamic_cast<GetLibraryInfoBaton *>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(LibraryInfo(baton->libraryInfo).ToJs());

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::GetDeviceInfo)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new GetDeviceInfoBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<GetDeviceInfoBaton *>(b);
        return NRFJPROG_get_device_info(b->probe, &baton->deviceInfo);
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        const auto baton = dynamic_cast<GetDeviceInfoBaton *>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(DeviceInfo(baton->deviceInfo).ToJs());

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::GetReadbackProtection)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new GetReadbackProtectionBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<GetReadbackProtectionBaton *>(b);
        return NRFJPROG_get_readback_protection(b->probe, &baton->readbackProtection);
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        const auto baton = dynamic_cast<GetReadbackProtectionBaton *>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(Convert::toJsNumber(baton->readbackProtection));

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::Read)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton = std::make_unique<ReadBaton>();

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->length = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<ReadBaton *>(b);
        baton->data.resize(baton->length, 0);
        return NRFJPROG_read(b->probe, baton->address, baton->data.data(), baton->length);
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<ReadBaton *>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(Convert::toJsValueArray(baton->data.data(), baton->length));

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::ReadU32)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton     = std::make_unique<ReadU32Baton>();
        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<ReadU32Baton *>(b);
        return NRFJPROG_read_u32(b->probe, baton->address, &baton->data);
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        const auto baton = dynamic_cast<ReadU32Baton *>(b);
        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(Convert::toJsNumber(baton->data));

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::Program)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        std::unique_ptr<ProgramBaton> baton(new ProgramBaton());

        baton->file = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        const v8::Local<v8::Object> programOptions = Convert::getJsObject(parameters[argumentCount]);
        const ProgramOptions options(programOptions);
        baton->options     = options.options;
        baton->inputFormat = options.inputFormat;
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<ProgramBaton *>(b);

        FileFormatHandler file(baton->file, baton->inputFormat);

        if (!file.exists())
        {
            log(file.errormessage() + "\n");
            return INVALID_PARAMETER;
        }

        baton->filename = file.getFileName();

        auto programResult = NRFJPROG_program(b->probe, baton->filename.c_str(), baton->options);

        if (programResult == NOT_AVAILABLE_BECAUSE_PROTECTION && baton->options.chip_erase_mode == ERASE_ALL)
        {
            const nrfjprogdll_err_t recoverResult = NRFJPROG_recover(b->probe);

            if (recoverResult == SUCCESS)
            {
                programResult = NRFJPROG_program(b->probe, baton->filename.c_str(), baton->options);
            }
            else
            {
                programResult = recoverResult;
            }
        }

        return programResult;
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::ProgramDFU)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton      = std::make_unique<ProgramDFUBaton>();
        baton->filename = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        const auto baton = dynamic_cast<ProgramDFUBaton *>(b);

        FileFormatHandler file(baton->filename, INPUT_FORMAT_HEX_FILE);

        if (!file.exists())
        {
            log(file.errormessage() + "\n");
            return INVALID_PARAMETER;
        }

        const std::string filename = file.getFileName();
        program_options_t options;
        options.verify          = VERIFY_HASH;
        options.chip_erase_mode = ERASE_NONE;
        options.qspi_erase_mode = ERASE_NONE;
        options.reset           = RESET_NONE;

        return NRFJPROG_program(b->probe, filename.c_str(), options);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::ProgramMcuBootDFU)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton = std::make_unique<ProgramMcuBootDFUBaton>();

        baton->filename = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        baton->uart = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        baton->baudRate = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->responseTimeout = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        const auto baton = dynamic_cast<ProgramMcuBootDFUBaton *>(b);
        nrfjprogdll_err_t programResult;

        FileFormatHandler file(baton->filename, INPUT_FORMAT_HEX_FILE);

        if (!file.exists())
        {
            log(file.errormessage() + "\n");
            return INVALID_PARAMETER;
        }

        const std::string filename = file.getFileName();
        program_options_t options;
        options.verify          = VERIFY_NONE;
        options.chip_erase_mode = ERASE_NONE;
        options.qspi_erase_mode = ERASE_NONE;
        options.reset           = RESET_SYSTEM;

        programResult = NRFJPROG_program(b->probe, filename.c_str(), options);
        return programResult;
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::ProgramModemUartDFU)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton = std::make_unique<ProgramModemUartDFUBaton>();

        baton->filename = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        baton->uart = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        baton->baudRate = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->responseTimeout = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        const auto baton = dynamic_cast<ProgramModemUartDFUBaton *>(b);
        nrfjprogdll_err_t programResult;

        FileFormatHandler file(baton->filename, INPUT_FORMAT_HEX_FILE);

        if (!file.exists())
        {
            log(file.errormessage() + "\n");
            return INVALID_PARAMETER;
        }

        const std::string filename = file.getFileName();
        program_options_t options;
        options.verify          = VERIFY_NONE;
        options.chip_erase_mode = ERASE_NONE;
        options.qspi_erase_mode = ERASE_NONE;
        options.reset           = RESET_SYSTEM;

        programResult = NRFJPROG_program(b->probe, filename.c_str(), options);
        return programResult;
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::ReadToFile)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton = std::make_unique<ReadToFileBaton>();

        baton->filename = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        const v8::Local<v8::Object> readOptions = Convert::getJsObject(parameters[argumentCount]);
        const ReadToFileOptions options(readOptions);
        baton->options = options.options;
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        const auto baton = dynamic_cast<ReadToFileBaton *>(b);
        return NRFJPROG_read_to_file(b->probe, baton->filename.c_str(), baton->options);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Verify)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton = std::make_unique<VerifyBaton>();

        baton->filename = Convert::getNativeString(parameters[argumentCount]);
        argumentCount++;

        // There are no verify options at the moment, but there will be options
        // (like the option that the incomming content may be a string)
        Convert::getJsObject(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        const auto baton = dynamic_cast<VerifyBaton *>(b);
        return NRFJPROG_verify(b->probe, baton->filename.c_str(), VERIFY_READ);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Erase)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton                               = std::make_unique<EraseBaton>();
        const v8::Local<v8::Object> eraseOptions = Convert::getJsObject(parameters[argumentCount]);
        const EraseOptions options(eraseOptions);
        baton->erase_mode    = options.eraseMode;
        baton->start_address = options.startAddress;
        baton->end_address   = options.endAddress;
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        const auto baton = dynamic_cast<EraseBaton *>(b);
        return NRFJPROG_erase(b->probe, baton->erase_mode, baton->start_address, baton->end_address);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Recover)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new RecoverBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        return NRFJPROG_recover(b->probe);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Reset)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new ResetBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        return NRFJPROG_reset(b->probe, RESET_SYSTEM);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::Write)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton = std::make_unique<WriteBaton>();

        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->data   = Convert::getVectorForUint8(parameters[argumentCount]);
        baton->length = Convert::getLengthOfArray(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<WriteBaton *>(b);
        return NRFJPROG_write(b->probe, baton->address, baton->data.data(), baton->length);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::WriteU32)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton     = std::make_unique<WriteU32Baton>();
        baton->address = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        baton->data = Convert::getNativeUint32(parameters[argumentCount]);
        argumentCount++;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        const auto baton = dynamic_cast<WriteU32Baton *>(b);
        return NRFJPROG_write_u32(b->probe, baton->address, baton->data);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::OpenDevice)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new OpenBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        return pHighlvlStatic->registerProbe(b->serialNumber, b->probe);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::CloseDevice)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new CloseBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        pHighlvlStatic->unregisterProbe(b->serialNumber);
        return SUCCESS;
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::RttStart)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE parameters, int & argumentCount) -> Baton * {
        auto baton = std::make_unique<RTTStartBaton>();

        const auto startOptions = Convert::getJsObject(parameters[argumentCount]);
        const StartOptions options(startOptions);
        baton->hasControlBlockLocation = options.hasControlBlockLocation;
        baton->controlBlockLocation    = options.controlBlockLocation;
        ++argumentCount;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<RTTStartBaton *>(b);

        if (pHighlvlStatic->hasProbe(baton->serialNumber))
        {
            return INVALID_OPERATION; // Already opened
        }

        {
            const auto result = NRFJPROG_reset(b->probe, RESET_SYSTEM);
            if (result != SUCCESS)
            {
                return result;
            }
        }

        if (baton->hasControlBlockLocation)
        {
            const auto result = NRFJPROG_rtt_set_control_block_address(b->probe, baton->controlBlockLocation);

            if (result != SUCCESS)
            {
                rttCleanup(b->probe);
                return result;
            }
        }

        pHighlvlStatic->rttStartTime = std::chrono::high_resolution_clock::now();

        const auto result = NRFJPROG_rtt_start(b->probe);

        if (result != SUCCESS)
        {
            rttCleanup(b->probe);
            return result;
        }

        const auto waitStatus = waitForControlBlock(baton->probe, baton->foundControlBlock);

        if (waitStatus != SUCCESS)
        {
            rttCleanup(b->probe);
            return waitStatus;
        }

        const auto channelInfo = getChannelInformation(baton, baton->foundChannelInformation);

        if (channelInfo != SUCCESS)
        {
            return channelInfo;
        }

        // Add to registry to keep it open
        return pHighlvlStatic->registerProbe(b->serialNumber, b->probe);
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<RTTStartBaton *>(b);

        std::vector<v8::Local<v8::Value>> returnData;

        v8::Local<v8::Array> downChannelInfo = Nan::New<v8::Array>();
        int i                                = 0;
        for (auto & element : baton->downChannelInfo)
        {
            Nan::Set(downChannelInfo, Convert::toJsNumber(i), element->ToJs());
            ++i;
        }

        returnData.emplace_back(downChannelInfo);

        v8::Local<v8::Array> upChannelInfo = Nan::New<v8::Array>();
        i                                  = 0;
        for (auto & element : baton->upChannelInfo)
        {
            Nan::Set(upChannelInfo, Convert::toJsNumber(i), element->ToJs());
            ++i;
        }

        returnData.emplace_back(upChannelInfo);

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::RttStop)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int &) -> Baton * {
        return new RTTStopBaton();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        return rttCleanup(b->probe);
    };

    CallFunction(info, p, e, nullptr, true);
}

NAN_METHOD(HighLevel::RttRead)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int & argumentCount) -> Baton * {
        auto baton = std::make_unique<RTTReadBaton>();

        baton->channelIndex = Convert::getNativeUint32(info[argumentCount]);
        ++argumentCount;

        baton->length = Convert::getNativeUint32(info[argumentCount]);
        ++argumentCount;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<RTTReadBaton *>(b);

        if (!isRttStarted(b->probe))
        {
            baton->rttNotStarted = true;
            return INTERNAL_ERROR;
        }

        baton->data.resize(baton->length, 0);
        uint32_t readLength = 0;

        baton->functionStart = std::chrono::high_resolution_clock::now();

        const auto status =
            NRFJPROG_rtt_read(b->probe, baton->channelIndex, baton->data.data(), baton->length, &readLength);

        if (status != SUCCESS)
        {
            rttCleanup(b->probe);
            return status;
        }

        baton->length = readLength;

        return status;
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        auto baton = dynamic_cast<RTTReadBaton *>(b);

        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(Convert::toJsString(baton->data.data(), baton->length));
        returnData.emplace_back(
            Convert::toJsValueArray(reinterpret_cast<uint8_t *>(baton->data.data()), baton->length));
        returnData.emplace_back(Convert::toTimeDifferenceUS(pHighlvlStatic->rttStartTime, baton->functionStart));

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}

NAN_METHOD(HighLevel::RttWrite)
{
    const parse_parameters_function_t p = [&](Nan::NAN_METHOD_ARGS_TYPE, int & argumentCount) -> Baton * {
        auto baton = std::make_unique<RTTWriteBaton>();

        baton->channelIndex = Convert::getNativeUint32(info[argumentCount]);
        ++argumentCount;

        if (info[argumentCount]->IsString())
        {
            baton->data   = Convert::getVectorForChar(info[argumentCount]);
            baton->length = static_cast<uint32_t>(Convert::getNativeString(info[argumentCount]).length());
        }
        else
        {
            const auto tempData = Convert::getVectorForUint8(info[argumentCount]);
            baton->data         = std::vector<char>(tempData.begin(), tempData.end());
            baton->length       = Convert::getLengthOfArray(info[argumentCount]);
        }
        ++argumentCount;

        return baton.release();
    };

    const execute_function_t e = [&](Baton * b) -> nrfjprogdll_err_t {
        auto baton = dynamic_cast<RTTWriteBaton *>(b);

        if (!isRttStarted(b->probe))
        {
            baton->rttNotStarted = true;
            return INTERNAL_ERROR;
        }

        uint32_t writeLength = 0;

        baton->functionStart = std::chrono::high_resolution_clock::now();

        const auto status =
            NRFJPROG_rtt_write(b->probe, baton->channelIndex, baton->data.data(), baton->length, &writeLength);

        if (status != SUCCESS)
        {
            rttCleanup(b->probe);
            return status;
        }

        baton->length = writeLength;
        return status;
    };

    const return_function_t r = [&](Baton * b) -> std::vector<v8::Local<v8::Value>> {
        const auto baton = dynamic_cast<RTTWriteBaton *>(b);

        std::vector<v8::Local<v8::Value>> returnData;

        returnData.emplace_back(Convert::toJsNumber(baton->length));
        returnData.emplace_back(Convert::toTimeDifferenceUS(pHighlvlStatic->rttStartTime, baton->functionStart));

        return returnData;
    };

    CallFunction(info, p, e, r, true);
}
