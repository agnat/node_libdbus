#ifndef NODE_DBUS_TIMEOUT_INCLUDED
#define NODE_DBUS_TIMEOUT_INCLUDED

#include <dbus/dbus.h>

#include "v8_utils.hpp"

namespace node_dbus {

class Timeout : public v8_utils::Wrapped<Timeout> {
        typedef v8_utils::Wrapped<Timeout> base;
    public:
        static void Initialize(v8_utils::ObjectHandle exports);

        static Timeout * New(DBusTimeout * t);

        inline DBusTimeout * timeout() { return timeout_; }
    private:
        Timeout();
        Timeout(DBusTimeout * t);

        static v8::Handle<v8::Value> New(v8::Arguments const&);
        static v8::Handle<v8::Value> Handle(v8::Arguments const&);

        static
        v8::Handle<v8::Value>
        GetInterval(v8::Local<v8::String> property, const v8::AccessorInfo &info);

        DBusTimeout * timeout_;
};

} // end of namespace node_dbus
#endif // NODE_DBUS_TIMEOUT_INCLUDED
