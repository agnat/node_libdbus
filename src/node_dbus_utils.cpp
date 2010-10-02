#include "node_dbus_utils.hpp"

#include <sstream>
#include <dbus/dbus.h>

#include "v8_utils.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

NodeDBusError::NodeDBusError() {
    dbus_error_init(&error_);
}

NodeDBusError::~NodeDBusError() {
    if (dbus_error_is_set(&error_)) {
        dbus_error_free(&error_);
    }
}

v8::Handle<v8::Value>
NodeDBusError::exception() const {
    std::ostringstream msg;
    msg << "DBus error: " << error_.message;
    return throwError(msg.str().c_str());
}

} // end of namespace node_dbus
