#include "node_dbus_connection.hpp"
#include "node_dbus_watch.hpp"

#include <dbus/dbus.h>

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

void
init(Handle<Object> ex) {
    ObjectHandle exports(ex);
    Connection::Initialize(exports);
    Watch::Initialize(exports);

    Handle<Object> constants = Object::New();
    exports->Set(String::NewSymbol("constants"), constants);

    NODE_DEFINE_CONSTANT(constants, DBUS_BUS_SESSION);
    NODE_DEFINE_CONSTANT(constants, DBUS_BUS_SYSTEM);
    NODE_DEFINE_CONSTANT(constants, DBUS_BUS_STARTER);

    NODE_DEFINE_CONSTANT(constants, DBUS_WATCH_READABLE);
    NODE_DEFINE_CONSTANT(constants, DBUS_WATCH_WRITABLE);
    NODE_DEFINE_CONSTANT(constants, DBUS_WATCH_ERROR);
    NODE_DEFINE_CONSTANT(constants, DBUS_WATCH_HANGUP);
}

} // end of namespace node_dbus

extern "C"
void
init(Handle<Object> exports) {
    node_dbus::init(exports);
}
