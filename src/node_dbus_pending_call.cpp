#include "node_dbus_pending_call.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

PendingCall::PendingCall(DBusPendingCall * call) :
      base()
    , pending_call_(call)
{}

void
PendingCall::Initialize(ObjectHandle exports) {
    base::Initialize("PendingCall", New);

    prototype_method("setNotify", SetNotifiy);
    prototype_method("cancel", Cancel);

    property("completed", GetCompleted);

    exports["PendingCall"] = function();
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

    return args.This();
}

Handle<Value>
PendingCall::SetNotifiy(Arguments const& args) {
    HandleScope scope;
    return Undefined();
}

Handle<Value>
PendingCall::Cancel(Arguments const& args) {
    HandleScope scope;
    return Undefined();
}

v8::Handle<v8::Value>
PendingCall::GetCompleted(v8::Local<v8::String> property, const AccessorInfo &info) {
    HandleScope scope;
    PendingCall * pc = unwrap(info.Holder());
    return scope.Close(Boolean::New((dbus_pending_call_get_completed(pc->pending_call()))));
}

} // end of namespace node_dbus
