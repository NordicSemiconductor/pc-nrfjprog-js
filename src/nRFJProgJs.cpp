#include <nan.h>

#include <vector>

#include "common.h"
#include "nrfjprog_common.h"
#include "nRFJProgJs.h"

Nan::Persistent<v8::Function> DebugProbe::constructor;

class ProbeInfo
{
public:
	ProbeInfo(uint32_t serial_number, device_family_t family) :
		serial_number(serial_number), family(family) {}

	uint32_t serial_number;
	device_family_t family;
};

static std::vector<ProbeInfo*> probes;

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

DebugProbe::DebugProbe()
{
}

DebugProbe::~DebugProbe()
{
}

void DebugProbe::init(v8::Local<v8::FunctionTemplate> tpl)
{
    Nan::SetPrototypeMethod(tpl, "connect", Connect);
}

#pragma region Connect
NAN_METHOD(DebugProbe::Connect)
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

	auto baton = new ConnectBaton(callback);

    uv_queue_work(uv_default_loop(), baton->req, Connect, reinterpret_cast<uv_after_work_cb>(AfterConnect));
}


void DebugProbe::Connect(uv_work_t *req)
{
    //auto baton = static_cast<x*>(req->data);
    //baton->result =
}

void DebugProbe::AfterConnect(uv_work_t *req)
{
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[1];

    argv[0] = Nan::Undefined();

    auto baton = static_cast<ConnectBaton *>(req->data);
    baton->callback->Call(0, argv);

    delete baton;
}

#pragma endregion Connect

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

		auto const probe_count_max = 40;
		uint32_t probe_count;

		uint32_t _probes[probe_count_max];

		// Find nRF51 devices available
		NRFJPROG_open_dll("JLinkARM.dll", nullptr, NRF51_FAMILY);
		NRFJPROG_enum_emu_snr(_probes, probe_count_max, &probe_count);
		NRFJPROG_close_dll();

		for (auto i = 0; i < probe_count; i++)
		{
			probes.push_back(new ProbeInfo(_probes[i], NRF51_FAMILY));
		}

		// Find nRF52 devices available
		NRFJPROG_open_dll("jlinkarm_nrf52_nrfjprog.dll", nullptr, NRF52_FAMILY);
		NRFJPROG_close_dll();
    }
}

NODE_MODULE(pc_nrfjprog, init);
