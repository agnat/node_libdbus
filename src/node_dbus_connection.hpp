#ifndef NODE_DBUS_CONNECTION_INCLUDED
#define NODE_DBUS_CONNECTION_INCLUDED

#include "v8_utils.hpp"

struct DBusConnection;

namespace node_dbus {

class Connection : public v8_utils::Wrapped<Connection> {
        typedef v8_utils::Wrapped<Connection> base;
    public:
        static void Initialize(v8_utils::ObjectHandle exports);

        ~Connection();
    private:
        Connection();
        Connection(DBusConnection * connection);
        void close();

        static v8::Handle<v8::Value> New(v8::Arguments const&);
        static v8::Handle<v8::Value> Close(v8::Arguments const&);

        DBusConnection * connection_;
        bool             closed_;
};

} // end of namespace node_dbus

#endif // NODE_DBUS_CONNECTION_INCLUDED
