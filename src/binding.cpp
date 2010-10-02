#include "node_dbus_connection.hpp"
#include "node_dbus_watch.hpp"
#include "node_dbus_timeout.hpp"
#include "node_dbus_message.hpp"
#include "node_dbus_pending_call.hpp"

#include <dbus/dbus.h>

using namespace v8;
using namespace v8_utils;

namespace v8_utils {

template <> struct convert_to_js<DBusBusType> : convert_to_js<int> {};
template <> struct convert_to_js<DBusWatchFlags> : convert_to_js<int> {};
template <> struct convert_to_js<DBusDispatchStatus> : convert_to_js<int> {};

}

namespace node_dbus {

template <typename T>
inline
void
defineConstant(Handle<Object> exports, const char * name, T value) {
    typedef convert_to_js<T> convert_to_js;
    exports->Set(String::NewSymbol(name),
                convert_to_js::convert(value),
                static_cast<PropertyAttribute>(ReadOnly|DontDelete));
}

inline
void
defineStringConstant(Handle<Object> exports,
        const char * name, const char * value)
{
    exports->Set(String::NewSymbol(name),
                String::New(value),
                static_cast<PropertyAttribute>(ReadOnly|DontDelete));
}

#define STRING_CONSTANT(ex, C) defineStringConstant(ex, #C, C)
#define DEFINE_CONSTANT(ex, C) defineConstant(ex, #C, C)

void 
init_constants(ObjectHandle exports) {
    Handle<Object> constants = Object::New();
    exports->Set(String::NewSymbol("constants"), constants);

    DEFINE_CONSTANT(constants, DBUS_BUS_SESSION);
    DEFINE_CONSTANT(constants, DBUS_BUS_SYSTEM);
    DEFINE_CONSTANT(constants, DBUS_BUS_STARTER);

    DEFINE_CONSTANT(constants, DBUS_WATCH_READABLE);
    DEFINE_CONSTANT(constants, DBUS_WATCH_WRITABLE);
    DEFINE_CONSTANT(constants, DBUS_WATCH_ERROR);
    DEFINE_CONSTANT(constants, DBUS_WATCH_HANGUP);

    DEFINE_CONSTANT(constants, DBUS_MESSAGE_TYPE_INVALID);
    DEFINE_CONSTANT(constants, DBUS_MESSAGE_TYPE_METHOD_CALL);
    DEFINE_CONSTANT(constants, DBUS_MESSAGE_TYPE_METHOD_RETURN);
    DEFINE_CONSTANT(constants, DBUS_MESSAGE_TYPE_ERROR);
    DEFINE_CONSTANT(constants, DBUS_MESSAGE_TYPE_SIGNAL);

    DEFINE_CONSTANT(constants, DBUS_SERVICE_DBUS);
    DEFINE_CONSTANT(constants, DBUS_PATH_DBUS);
    DEFINE_CONSTANT(constants, DBUS_INTERFACE_DBUS);

    DEFINE_CONSTANT(constants, DBUS_DISPATCH_DATA_REMAINS);
    DEFINE_CONSTANT(constants, DBUS_DISPATCH_COMPLETE);
    DEFINE_CONSTANT(constants, DBUS_DISPATCH_NEED_MEMORY);
}

void
init(Handle<Object> ex) {
    ObjectHandle exports(ex);

    Connection::Initialize(exports);
    Watch::Initialize(exports);
    Timeout::Initialize(exports);
    Message::Initialize(exports);
    PendingCall::Initialize(exports);

    defineFunction(exports, "createMethodCall", Message::CreateMethodCall);
    
    init_constants(exports);
}

} // end of namespace node_dbus

extern "C"
void
init(Handle<Object> exports) {
    node_dbus::init(exports);
}
