#ifndef NODE_DBUS_DBUS_UTILS_INCLUDED
#define NODE_DBUS_DBUS_UTILS_INCLUDED

#include <iostream>
#include <node.h>
#include <dbus/dbus.h>

namespace node_dbus {

class NodeDBusError {
    public:
        NodeDBusError();
        ~NodeDBusError();

        operator DBusError*() { return & error_; }
        operator bool() { return dbus_error_is_set(& error_); }

        v8::Handle<v8::Value> exception() const;
    private:
        DBusError error_;
};

} // end of namespace node_dbus

#endif // NODE_DBUS_DBUS_UTILS_INCLUDED
