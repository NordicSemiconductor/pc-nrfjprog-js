#ifndef __NRFJPROG_H__
#define __NRFJPROG_H__

#include <nan.h>
#include "common.h"
#include "dllfunc.h"
#include "osfiles.h"

#define MAX_SERIAL_NUMBERS 100

#define NRFJPROGJS_METHOD_DEFINITIONS(MainName) \
    static NAN_METHOD(MainName); \
    static void MainName(uv_work_t *req); \
    static void After##MainName(uv_work_t *req);

class ProbeInfo
{
public:
    ProbeInfo(uint32_t serial_number, device_family_t family) :
        serial_number(serial_number), family(family)
    {}

    uint32_t serial_number;
    device_family_t family;

    v8::Local<v8::Object> ToJs();
};

class DebugProbe : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);

private:
    explicit DebugProbe();
    ~DebugProbe();

    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);

    // Sync methods

    // Async methods
    NRFJPROGJS_METHOD_DEFINITIONS(Program); // Params: Serialnumber, family, file, callback
    NRFJPROGJS_METHOD_DEFINITIONS(GetSerialnumbers); // Params: None, callback
    NRFJPROGJS_METHOD_DEFINITIONS(GetVersion); // Params: Serialnumber, family, callback

    static void init(v8::Local<v8::FunctionTemplate> tpl);

    static device_family_t getFamily(const uint32_t serialnumber);

    static uint32_t emulatorSpeed;
    static DllFunctionPointersType dll_function;
    static char dll_path[COMMON_MAX_PATH];
    static char jlink_path[COMMON_MAX_PATH];

    static bool loaded;
    static int error;
};

class ConnectBaton : public Baton {
public:
    BATON_CONSTRUCTOR(ConnectBaton);
};

class DisconnectBaton : public Baton {
public:
    BATON_CONSTRUCTOR(DisconnectBaton);
};

class ProgramBaton : public Baton {
public:
    BATON_CONSTRUCTOR(ProgramBaton);
    uint32_t serialnumber;
    device_family_t family;
    std::string filename;
    std::map<device_family_t, std::string> filenameMap;
    bool useProvidedFamily;
};

class GetSerialnumbersBaton : public Baton {
public:
    BATON_CONSTRUCTOR(GetSerialnumbersBaton);
    std::vector<ProbeInfo*> probes;
};

class GetVersionBaton : public Baton {
public:
    BATON_CONSTRUCTOR(GetVersionBaton);
    uint32_t serialnumber;
    device_family_t family;
    uint32_t version;
};

#endif // __NRFJPROG_H__
