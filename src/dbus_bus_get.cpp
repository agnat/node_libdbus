#include <node.h>

namespace node_dbus {

v8::Handle<v8::Value>
dbus_bus_get(v8::Arguments const& args) {
    HandleScope scope;

    DBusConnection *
    return scope.Close(Connection::New(connection));
}
