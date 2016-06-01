#include <nan.h>

#include <vector>

#include "nRFJProgJs.h"
#include "nrfjprog_common.h"

#include "KeilHexFile.h"

#include <iostream>

Nan::Persistent<v8::Function> DebugProbe::constructor;

v8::Local<v8::Object> ProbeInfo::ToJs()
{
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Object> obj = Nan::New<v8::Object>();

    Utility::Set(obj, "serialNumber", ConversionUtility::toJsNumber(serial_number));
    Utility::Set(obj, "family", ConversionUtility::toJsNumber(family));

    return scope.Escape(obj);
}

NAN_MODULE_INIT(DebugProbe::Init)
{
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("DebugProbe").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    init(tpl);

    constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("DebugProbe").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(DebugProbe::New)
{
    if (info.IsConstructCall()) {
        auto obj = new DebugProbe();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    } else {
        v8::Local<v8::Function> cons = Nan::New(constructor);
        info.GetReturnValue().Set(cons->NewInstance());
    }
}

uint32_t DebugProbe::emulatorSpeed = 1000;

DebugProbe::DebugProbe()
{
}

DebugProbe::~DebugProbe()
{
}

void DebugProbe::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "program", Program);
    Nan::SetPrototypeMethod(tpl, "getVersion", GetVersion);
    Nan::SetPrototypeMethod(tpl, "getSerialNumbers", GetSerialnumbers);
}

device_family_t DebugProbe::getFamily(const uint32_t serialnumber)
{
    device_version_t deviceType = UNKNOWN;

    NRFJPROG_connect_to_emu_with_snr(serialnumber, emulatorSpeed);

    NRFJPROG_read_device_version(&deviceType);

    NRFJPROG_disconnect_from_emu();

    switch (deviceType)
    {
        case NRF51_XLR1:
        case NRF51_XLR2:
        case NRF51_XLR3:
        case NRF51_L3:
        case NRF51_XLR3P:
        case NRF51_XLR3LC:
            return NRF51_FAMILY;
        case NRF52_FP1_ENGA:
        case NRF52_FP1_ENGB:
        case NRF52_FP1:
        case UNKNOWN:
        default:
            return NRF52_FAMILY;
    }
}

#pragma region GetSerialnumbers
NAN_METHOD(DebugProbe::GetSerialnumbers)
{
    auto obj = Nan::ObjectWrap::Unwrap<DebugProbe>(info.Holder());
    auto argumentCount = 0;
	v8::Local<v8::Function> callback;

    try
    {
		callback = ConversionUtility::getCallbackFunction(info[argumentCount]);
		argumentCount++;
    }
    catch (std::string error)
    {
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);
        return;
    }

    auto baton = new GetSerialnumbersBaton(callback);

    uv_queue_work(uv_default_loop(), baton->req, GetSerialnumbers, reinterpret_cast<uv_after_work_cb>(AfterGetSerialnumbers));
}


void DebugProbe::GetSerialnumbers(uv_work_t *req)
{
    auto baton = static_cast<GetSerialnumbersBaton*>(req->data);
    auto const probe_count_max = MAX_SERIAL_NUMBERS;
    uint32_t probe_count = 0;

    uint32_t _probes[MAX_SERIAL_NUMBERS];

    // Find nRF51 devices available
    baton->result = NRFJPROG_open_dll("JLinkARM.dll", nullptr, NRF51_FAMILY);

    if (baton->result != SUCCESS)
    {
        return;
    }

    baton->result = NRFJPROG_enum_emu_snr(_probes, probe_count_max, &probe_count);

    if (baton->result != SUCCESS)
    {
        return;
    }

    for (uint32_t i = 0; i < probe_count; i++)
    {
        device_family_t family = getFamily(_probes[i]);

        baton->probes.push_back(new ProbeInfo(_probes[i], family));
    }

    NRFJPROG_close_dll();
}

void DebugProbe::AfterGetSerialnumbers(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<GetSerialnumbersBaton*>(req->data);
    v8::Local<v8::Value> argv[2];

    if (baton->result != SUCCESS)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "getting serialnumbers");
        argv[1] = Nan::Undefined();
    }
    else
    {
        argv[0] = Nan::Undefined();
        v8::Local<v8::Array> serialNumbers = Nan::New<v8::Array>();

        for (uint32_t i = 0; i < baton->probes.size(); ++i)
        {
            Nan::Set(serialNumbers, Nan::New<v8::Integer>(i), baton->probes[i]->ToJs());
        }

        argv[1] = serialNumbers;
    }

    baton->callback->Call(2, argv);

    delete baton;
}
#pragma endregion GetSerialnumbers

#pragma region Program
NAN_METHOD(DebugProbe::Program)
{
    auto obj = Nan::ObjectWrap::Unwrap<DebugProbe>(info.Holder());
    auto argumentCount = 0;

    uint32_t serialNumber;
    uint32_t family = ANY_FAMILY;
    std::string filename;
    v8::Local<v8::Object> filenameObject;
    v8::Local<v8::Function> callback;

    try
    {
        serialNumber = ConversionUtility::getNativeUint32(info[argumentCount]);
        argumentCount++;
        
        if (info.Length() == 3)
        {
            filenameObject = ConversionUtility::getJsObject(info[argumentCount]);
            argumentCount++;
        }
        else if (info.Length() == 4)
        {
            family = ConversionUtility::getNativeUint32(info[argumentCount]);
            argumentCount++;
                        
            filename = ConversionUtility::getNativeString(info[argumentCount]);
            argumentCount++;
        }
        else
        {
            throw "parameter count";
        }

        callback = ConversionUtility::getCallbackFunction(info[argumentCount]);
        argumentCount++;
    }
    catch (std::string error)
    {
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);
        return;
    }

    

    auto baton = new ProgramBaton(callback);
    baton->serialnumber = serialNumber;
    baton->family = (device_family_t)family;

    if (info.Length() == 4)
    {
        std::cout << "Specific " << filename << std::endl;
        baton->filename = filename;
    }
    else
    {
        if (Utility::Has(filenameObject, NRF51_FAMILY))
        {
            baton->filenameMap[NRF51_FAMILY] = ConversionUtility::getNativeString(Utility::Get(filenameObject, NRF51_FAMILY));
            std::cout << "51: " << baton->filenameMap[NRF51_FAMILY] << std::endl;
        }

        if (Utility::Has(filenameObject, NRF52_FAMILY))
        {
            baton->filenameMap[NRF52_FAMILY] = ConversionUtility::getNativeString(Utility::Get(filenameObject, NRF52_FAMILY));
            std::cout << "52: " << baton->filenameMap[NRF52_FAMILY] << std::endl;
        }
    }

    uv_queue_work(uv_default_loop(), baton->req, Program, reinterpret_cast<uv_after_work_cb>(AfterProgram));
}

void DebugProbe::Program(uv_work_t *req)
{
    auto baton = static_cast<ProgramBaton*>(req->data);
    auto const probe_count_max = MAX_SERIAL_NUMBERS;
    uint32_t probe_count = 0;

    if (baton->family != ANY_FAMILY)
    {
        baton->result = NRFJPROG_open_dll("JLinkARM.dll", nullptr, baton->family);
        std::cout << "Specific family " << baton->family << std::endl;
    }
    else
    {
        baton->result = NRFJPROG_open_dll("JLinkARM.dll", nullptr, NRF51_FAMILY);
        baton->family = getFamily(baton->serialnumber);

        if (baton->family != NRF51_FAMILY)
        {
            NRFJPROG_close_dll();
            baton->result = NRFJPROG_open_dll("JLinkARM.dll", nullptr, baton->family);
        }

        baton->filename = baton->filenameMap[baton->family];
        std::cout << "Found family " << baton->family << std::endl;
    }

    if (baton->result != SUCCESS)
    {
        return;
    }

    baton->result = NRFJPROG_connect_to_emu_with_snr(baton->serialnumber, emulatorSpeed);

    if (baton->result != SUCCESS)
    {
        return;
    }

    KeilHexFile program_hex;

    KeilHexFile::Status status = program_hex.open(baton->filename.c_str());

    uint32_t code_size = 512 * 1024;
    uint8_t *code = new uint8_t[code_size];

    if (program_hex.nand_read(0, code, code_size) != KeilHexFile::SUCCESS)
    {
        baton->result = -2;
        //TODO: Set proper errorcode
        std::cout << "Nand read failed" << std::endl;
        NRFJPROG_close_dll();
        delete[] code;
        return;
    }

    if (NRFJPROG_erase_all() != SUCCESS)
    {
        baton->result = -1;
        std::cout << "Erase all failed" << std::endl;
        //TODO: Set proper errorcode
        NRFJPROG_close_dll();
        delete[] code;
        return;
    }

    uint32_t foundAddress;
    uint32_t bytesFound;

    program_hex.find_contiguous(0, foundAddress, bytesFound);

    do
    {
        baton->result = NRFJPROG_write(foundAddress, (const uint8_t *)&code[foundAddress], bytesFound, true);

        if (baton->result != SUCCESS)
        {
            //TODO: Set proper errorcode
            std::cout << "Write failed" << std::endl;
            break;
        }

        program_hex.find_contiguous(foundAddress + bytesFound, foundAddress, bytesFound);
    } while (bytesFound != 0);

    NRFJPROG_close_dll();
    delete[] code;
}

void DebugProbe::AfterProgram(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<ProgramBaton*>(req->data);
    v8::Local<v8::Value> argv[1];

    if (baton->result != SUCCESS)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "programming");
    }
    else
    {
        argv[0] = Nan::Undefined();
    }

    baton->callback->Call(1, argv);

    delete baton;
}
#pragma endregion Program

#pragma region GetVersion
NAN_METHOD(DebugProbe::GetVersion)
{
    auto obj = Nan::ObjectWrap::Unwrap<DebugProbe>(info.Holder());
    auto argumentCount = 0;

    uint32_t serialNumber;
    uint32_t family;
    v8::Local<v8::Function> callback;

    try
    {
        serialNumber = ConversionUtility::getNativeUint32(info[argumentCount]);
        argumentCount++;

        family = ConversionUtility::getNativeUint32(info[argumentCount]);
        argumentCount++;

        callback = ConversionUtility::getCallbackFunction(info[argumentCount]);
        argumentCount++;
    }
    catch (std::string error)
    {
        auto message = ErrorMessage::getTypeErrorMessage(argumentCount, error);
        Nan::ThrowTypeError(message);
        return;
    }

    auto baton = new GetVersionBaton(callback);
    baton->serialnumber = serialNumber;
    baton->family = (device_family_t)family;

    uv_queue_work(uv_default_loop(), baton->req, GetVersion, reinterpret_cast<uv_after_work_cb>(AfterGetVersion));
}


void DebugProbe::GetVersion(uv_work_t *req)
{
    auto baton = static_cast<GetVersionBaton*>(req->data);
    auto const probe_count_max = MAX_SERIAL_NUMBERS;
    uint32_t probe_count = 0;

    uint32_t _probes[MAX_SERIAL_NUMBERS];

    // Find nRF51 devices available
    baton->result = NRFJPROG_open_dll("JLinkARM.dll", nullptr, NRF51_FAMILY);

    if (baton->result != SUCCESS)
    {
        return;
    }

    baton->result = NRFJPROG_enum_emu_snr(_probes, probe_count_max, &probe_count);

    if (baton->result != SUCCESS)
    {
        return;
    }

    //TODO: Get version

    NRFJPROG_close_dll();
}

void DebugProbe::AfterGetVersion(uv_work_t *req)
{
    Nan::HandleScope scope;

    auto baton = static_cast<GetVersionBaton*>(req->data);
    v8::Local<v8::Value> argv[2];

    if (baton->result != SUCCESS)
    {
        argv[0] = ErrorMessage::getErrorMessage(baton->result, "getting version");
        argv[1] = Nan::Undefined();
    }
    else
    {
        argv[0] = Nan::Undefined();
        argv[1] = ConversionUtility::toJsNumber(baton->version);
    }

    baton->callback->Call(1, argv);

    delete baton;
}
#pragma endregion GetVersion

extern "C" {
    void initConsts(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
    {
        NODE_DEFINE_CONSTANT(target, R0);
        NODE_DEFINE_CONSTANT(target, R1);
        NODE_DEFINE_CONSTANT(target, R2);
        NODE_DEFINE_CONSTANT(target, R3);
        NODE_DEFINE_CONSTANT(target, R4);
        NODE_DEFINE_CONSTANT(target, R5);
        NODE_DEFINE_CONSTANT(target, R6);
        NODE_DEFINE_CONSTANT(target, R7);
        NODE_DEFINE_CONSTANT(target, R8);
        NODE_DEFINE_CONSTANT(target, R9);
        NODE_DEFINE_CONSTANT(target, R10);
        NODE_DEFINE_CONSTANT(target, R11);
        NODE_DEFINE_CONSTANT(target, R12);
        NODE_DEFINE_CONSTANT(target, R13);
        NODE_DEFINE_CONSTANT(target, R14);
        NODE_DEFINE_CONSTANT(target, R15);
        NODE_DEFINE_CONSTANT(target, XPSR);
        NODE_DEFINE_CONSTANT(target, MSP);
        NODE_DEFINE_CONSTANT(target, PSP);

        NODE_DEFINE_CONSTANT(target, RAM_OFF);
        NODE_DEFINE_CONSTANT(target, RAM_ON);

        NODE_DEFINE_CONSTANT(target, NONE);
        NODE_DEFINE_CONSTANT(target, REGION_0);
        NODE_DEFINE_CONSTANT(target, ALL);
        NODE_DEFINE_CONSTANT(target, BOTH);

        NODE_DEFINE_CONSTANT(target, NO_REGION_0);
        NODE_DEFINE_CONSTANT(target, FACTORY);
        NODE_DEFINE_CONSTANT(target, USER);

        NODE_DEFINE_CONSTANT(target, UNKNOWN);
        NODE_DEFINE_CONSTANT(target, NRF51_XLR1);
        NODE_DEFINE_CONSTANT(target, NRF51_XLR2);
        NODE_DEFINE_CONSTANT(target, NRF51_XLR3);
        NODE_DEFINE_CONSTANT(target, NRF51_L3);
        NODE_DEFINE_CONSTANT(target, NRF51_XLR3P);
        NODE_DEFINE_CONSTANT(target, NRF51_XLR3LC);
        NODE_DEFINE_CONSTANT(target, NRF52_FP1_ENGA);
        NODE_DEFINE_CONSTANT(target, NRF52_FP1_ENGB);
        NODE_DEFINE_CONSTANT(target, NRF52_FP1);

        NODE_DEFINE_CONSTANT(target, NRF51_FAMILY);
        NODE_DEFINE_CONSTANT(target, NRF52_FAMILY);

        NODE_DEFINE_CONSTANT(target, UP_DIRECTION);
        NODE_DEFINE_CONSTANT(target, DOWN_DIRECTION);

        NODE_DEFINE_CONSTANT(target, SUCCESS);
        NODE_DEFINE_CONSTANT(target, OUT_OF_MEMORY);
        NODE_DEFINE_CONSTANT(target, INVALID_OPERATION);
        NODE_DEFINE_CONSTANT(target, INVALID_PARAMETER);
        NODE_DEFINE_CONSTANT(target, INVALID_DEVICE_FOR_OPERATION);
        NODE_DEFINE_CONSTANT(target, WRONG_FAMILY_FOR_DEVICE);
        NODE_DEFINE_CONSTANT(target, EMULATOR_NOT_CONNECTED);
        NODE_DEFINE_CONSTANT(target, CANNOT_CONNECT);
        NODE_DEFINE_CONSTANT(target, LOW_VOLTAGE);
        NODE_DEFINE_CONSTANT(target, NO_EMULATOR_CONNECTED);
        NODE_DEFINE_CONSTANT(target, NVMC_ERROR);
        NODE_DEFINE_CONSTANT(target, NOT_AVAILABLE_BECAUSE_PROTECTION);
        NODE_DEFINE_CONSTANT(target, JLINKARM_DLL_NOT_FOUND);
        NODE_DEFINE_CONSTANT(target, JLINKARM_DLL_COULD_NOT_BE_OPENED);
        NODE_DEFINE_CONSTANT(target, JLINKARM_DLL_ERROR);
        NODE_DEFINE_CONSTANT(target, JLINKARM_DLL_TOO_OLD);
        NODE_DEFINE_CONSTANT(target, NRFJPROG_SUB_DLL_NOT_FOUND);
        NODE_DEFINE_CONSTANT(target, NRFJPROG_SUB_DLL_COULD_NOT_BE_OPENED);
        NODE_DEFINE_CONSTANT(target, NOT_IMPLEMENTED_ERROR);

        NODE_DEFINE_CONSTANT(target, Success);
        NODE_DEFINE_CONSTANT(target, NrfjprogError);
        NODE_DEFINE_CONSTANT(target, NrfjprogOutdatedError);
        NODE_DEFINE_CONSTANT(target, MemoryAllocationError);
        NODE_DEFINE_CONSTANT(target, InvalidArgumentError);
        NODE_DEFINE_CONSTANT(target, InsufficientArgumentsError);
        NODE_DEFINE_CONSTANT(target, IncompatibleArgumentsError);
        NODE_DEFINE_CONSTANT(target, DuplicatedArgumentsError);
        NODE_DEFINE_CONSTANT(target, NoOperationError);
        NODE_DEFINE_CONSTANT(target, UnavailableOperationBecauseProtectionError);
        NODE_DEFINE_CONSTANT(target, UnavailableOperationInFamilyError);
        NODE_DEFINE_CONSTANT(target, WrongFamilyForDeviceError);
        NODE_DEFINE_CONSTANT(target, NrfjprogDllNotFoundError);
        NODE_DEFINE_CONSTANT(target, NrfjprogDllLoadFailedError);
        NODE_DEFINE_CONSTANT(target, NrfjprogDllFunctionLoadFailedError);
        NODE_DEFINE_CONSTANT(target, NrfjprogDllNotImplementedError);
        NODE_DEFINE_CONSTANT(target, NrfjprogIniNotFoundError);
        NODE_DEFINE_CONSTANT(target, NrfjprogIniFormatError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllNotFoundError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllInvalidError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllFailedToOpenError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllError);
        NODE_DEFINE_CONSTANT(target, JLinkARMDllTooOldError);
        NODE_DEFINE_CONSTANT(target, InvalidSerialNumberError);
        NODE_DEFINE_CONSTANT(target, NoDebuggersError);
        NODE_DEFINE_CONSTANT(target, NotPossibleToConnectError);
        NODE_DEFINE_CONSTANT(target, LowVoltageError);
        NODE_DEFINE_CONSTANT(target, FileNotFoundError);
        NODE_DEFINE_CONSTANT(target, InvalidHexFileError);
        NODE_DEFINE_CONSTANT(target, FicrReadError);
        NODE_DEFINE_CONSTANT(target, WrongArgumentError);
        NODE_DEFINE_CONSTANT(target, VerifyError);
        NODE_DEFINE_CONSTANT(target, NoWritePermissionError);
        NODE_DEFINE_CONSTANT(target, NVMCOperationError);
        NODE_DEFINE_CONSTANT(target, FlashNotErasedError);
        NODE_DEFINE_CONSTANT(target, RamIsOffError);
        NODE_DEFINE_CONSTANT(target, FicrOperationWarning);
        NODE_DEFINE_CONSTANT(target, UnalignedPageEraseWarning);
        NODE_DEFINE_CONSTANT(target, NoLogWarning);
        NODE_DEFINE_CONSTANT(target, UicrWriteOperationWithoutEraseWarning);
    }

    NAN_MODULE_INIT(init)
    {
        initConsts(target);
        DebugProbe::Init(target);
    }
}

NODE_MODULE(pc_nrfjprog, init);
