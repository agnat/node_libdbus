#include "connection.hpp"

#include <iostream>

namespace node_dbus {

void
Connection::Initialize(v8::Handle<v8::Object> exports) {
    base::Initialize(exports, "Connection", New);
}

v8::Handle<v8::Value>
Connection::New(v8::Arguments const& args) {
    v8::HandleScope scope;
    
    std::cout << "Connection::New()" << std::endl;

    T * o = new T();
    o->Wrap(args.This());
    return args.This();
}

} // end of namespace node_dbus
