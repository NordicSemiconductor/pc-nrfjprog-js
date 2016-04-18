#ifndef __NRFJPROG_H__
#define __NRFJPROG_H__

#include <nan.h>

#define NRFJPROGJS_METHOD_DEFINITIONS(MainName) \
    static NAN_METHOD(MainName); \
    static void MainName(uv_work_t *req); \
    static void After##MainName(uv_work_t *req);


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
    NRFJPROGJS_METHOD_DEFINITIONS(Connect);
    //NRFJPROGJS_METHOD_DEFINITIONS(Disconnect);

    static void init(v8::Local<v8::FunctionTemplate> tpl);
};

class ConnectBaton : public Baton {
public:
    BATON_CONSTRUCTOR(ConnectBaton);

};

class DisconnectBaton : public Baton {
public:
    BATON_CONSTRUCTOR(DisconnectBaton);
};


#endif // __NRFJPROG_H__
