#include "connection.hpp"

//#include <dbus/dbus-shared.h>

namespace node_dbus {

void
init(v8::Handle<v8::Object> exports) {
    Connection::Initialize(exports);

    //NODE_DEFINE_CONSTANT(exports, DBUS_BUS_SESSION);
    //NODE_DEFINE_CONSTANT(exports, DBUS_BUS_SYSTEM);
    //NODE_DEFINE_CONSTANT(exports, DBUS_BUS_STARTER);
}

} // end of namespace node_dbus

extern "C"
void
init(v8::Handle<v8::Object> exports) {
    node_dbus::init(exports);
}
