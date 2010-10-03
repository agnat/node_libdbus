#include "node_dbus_timeout.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

Timeout::Timeout(DBusTimeout * t) :
      base()
    , timeout_(t)
{}

Timeout::~Timeout() {}

void
Timeout::Initialize(v8_utils::ObjectHandle exports) {
    base::Initialize("Timeout", New);

    prototype_method("handle", Handle);

    property("interval", GetInterval);

    exports["Timeout"] = function();
}

Timeout *
Timeout::New(DBusTimeout * timeout) {
    HandleScope scope;
    Local<Value> arg(External::New(timeout));
    return Timeout::unwrap(Timeout::function()->NewInstance(1, & arg));
}

Handle<Value>
Timeout::Handle(Arguments const& args) {
    HandleScope scope;
    Timeout * t =  Timeout::unwrap(args.This());
    return scope.Close(Integer::New(dbus_timeout_get_interval(t->timeout())));
}

Handle<Value>
Timeout::New(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }
    if ( ! args[0]->IsExternal()) {
        return throwTypeError("argument 1 must be an external (DBusTimeout)");
    }
    DBusTimeout * t = static_cast<DBusTimeout*>(External::Unwrap(args[0]));

    Timeout * o = new Timeout(t);
    o->Wrap(args.This());

    return args.This();
}

v8::Handle<v8::Value>
Timeout::GetInterval(v8::Local<v8::String> property, const AccessorInfo &info) {
    HandleScope scope;
    Timeout * t = unwrap(info.Holder());
    return scope.Close(Boolean::New((dbus_timeout_get_interval(t->timeout()))));
}

} // end of namespace node_dbus
