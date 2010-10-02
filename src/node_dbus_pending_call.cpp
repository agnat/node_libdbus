#include "node_dbus_pending_call.hpp"

#include <iostream>

#include "node_dbus_message.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

dbus_int32_t PendingCall::data_slot = -1;

PendingCall::PendingCall(DBusPendingCall * call) :
      base()
    , pending_call_(call)
{
}

void
PendingCall::Initialize(ObjectHandle exports) {
    base::Initialize("PendingCall", New);

    prototype_method("setNotify", SetNotifiy);
    prototype_method("cancel", Cancel);

    property("completed", GetCompleted);

    exports["PendingCall"] = function();

    if ( ! dbus_pending_call_allocate_data_slot( & data_slot)) {
        std::cerr << "failed to allocate data slot" << std::endl;
    }
}

PendingCall *
PendingCall::New(DBusPendingCall * pending_call) {
    HandleScope scope;
    Local<Value> arg = External::New(pending_call);
    return PendingCall::unwrap( PendingCall::function()->NewInstance(1, & arg) );
}

Handle<Value>
PendingCall::New(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }
    if ( ! args[0]->IsExternal()) {
        return throwTypeError("argument 1 must be an external (DBusWatch)");
    }
    DBusPendingCall * call = static_cast<DBusPendingCall*>(External::Unwrap(args[0]));

    PendingCall * o = new PendingCall(call);
    o->Wrap(args.This());

    dbus_pending_call_set_data(call, data_slot, o, NULL /*free*/);

    return args.This();
}

Handle<Value>
PendingCall::SetNotifiy(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }
    if ( ! args[0]->IsFunction()) {
        return throwTypeError("argument 1 must be a function");
    }
    PendingCall * pc = unwrap(args.This());
    pc->callback_ = Persistent<Function>::New(Local<Function>::Cast(args[0]));

    dbus_pending_call_set_notify(pc->pending_call(), OnResult, NULL, NULL /*free*/);

    return Undefined();
}

Handle<Value>
PendingCall::Cancel(Arguments const& args) {
    HandleScope scope;
    PendingCall * pc = unwrap(args.Holder());
    dbus_pending_call_cancel(pc->pending_call());
    return Undefined();
}

v8::Handle<v8::Value>
PendingCall::GetCompleted(v8::Local<v8::String> property, const AccessorInfo &info) {
    HandleScope scope;
    PendingCall * pc = unwrap(info.Holder());
    return scope.Close(Boolean::New((dbus_pending_call_get_completed(pc->pending_call()))));
}

void 
PendingCall::OnResult(DBusPendingCall * pc, void * data) {
    PendingCall * pending_call = static_cast<PendingCall*>(dbus_pending_call_get_data(pc, data_slot));
    if ( ! pending_call->callback_.IsEmpty()) {
        HandleScope scope;
        DBusMessage * m = dbus_pending_call_steal_reply(pending_call->pending_call());
        Local<Value> message(Local<Value>::New( Message::New(m)->handle_ ));
        pending_call->callback_->Call(pending_call->handle_, 1, & message);
    }
}
} // end of namespace node_dbus
