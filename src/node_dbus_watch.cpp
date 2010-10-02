#include "node_dbus_watch.hpp"

#include <iostream>

namespace node_dbus {

Watch::Watch(DBusWatch * watch) :
      watch_(watch)
{}

Watch::~Watch() {
    std::cout << "Watch::~Watch" << std::endl;
}

void
Watch::Initialize(v8_utils::ObjectHandle exports) {
    HandleScope scope;
    base::Initialize("Watch", New);
    
    prototype_method("flags",   Flags);
    prototype_method("unixFd",  UnixFd);
    prototype_method("enabled", Enabled);

    exports["Watch"] = function();
}

Watch *
Watch::New(DBusWatch * watch) {
    HandleScope scope;
    Local<Value> arg = External::New(watch);
    return node::ObjectWrap::Unwrap<Watch>(Watch::function()->NewInstance(1, &arg));
}

Handle<Value>
Watch::New(v8::Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }
    if ( ! args[0]->IsExternal()) {
        return throwTypeError("argument 1 must be an external (DBusWatch)");
    }
    DBusWatch * watch = static_cast<DBusWatch*>(External::Unwrap(args[0]));

    Watch * o = new Watch(watch);
    o->Wrap(args.This());

    return args.This();
}

v8::Handle<v8::Value>
Watch::Flags(v8::Arguments const& args) {
    return HandleScope().Close(
            Integer::New(dbus_watch_get_flags(Watch::unwrap(args.This())->watch())));
}

v8::Handle<v8::Value>
Watch::UnixFd(v8::Arguments const& args) {
    return HandleScope().Close(
            Integer::New(dbus_watch_get_unix_fd(Watch::unwrap(args.This())->watch())));
}

v8::Handle<v8::Value>
Watch::Enabled(v8::Arguments const& args) {
    return HandleScope().Close(
            Boolean::New(dbus_watch_get_enabled(Watch::unwrap(args.This())->watch())));
}

} // end of namespace node_dbus

