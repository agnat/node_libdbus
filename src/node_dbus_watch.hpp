#ifndef NODE_DBUS_DBUS_WATCH_INCLUDED
#define NODE_DBUS_DBUS_WATCH_INCLUDED

#include <dbus/dbus.h>

#include "v8_utils.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

class Watch : public v8_utils::Wrapped<Watch> {
        typedef v8_utils::Wrapped<Watch> base;
    public:
        static void Initialize(v8_utils::ObjectHandle exports);
        ~Watch();

        static Watch * New(DBusWatch * watch);

        inline DBusWatch * watch() { return watch_; }
    private:
        Watch();
        Watch(DBusWatch * watch);

        static v8::Handle<v8::Value> New(v8::Arguments const&);
        static v8::Handle<v8::Value> Flags(v8::Arguments const&);
        static v8::Handle<v8::Value> UnixFd(v8::Arguments const&);
        static v8::Handle<v8::Value> Enabled(v8::Arguments const&);

        DBusWatch * watch_;
};

}
// end of namespace node_dbus
#endif  // NODE_DBUS_DBUS_WATCH_INCLUDED

