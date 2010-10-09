#ifndef NODE_DBUS_CONNECTION_INCLUDED
#define NODE_DBUS_CONNECTION_INCLUDED

#include "v8_utils.hpp"

struct DBusConnection;

namespace node_dbus {

class Connection : public v8_utils::Wrapped<Connection> {
        typedef v8_utils::Wrapped<Connection> base;
    public:
        static void Initialize(v8_utils::ObjectHandle exports);

        inline DBusConnection * connection() { return connection_; }

        ~Connection();
    private:
        Connection();
        Connection(Connection const&);
        Connection const& operator=(Connection const&);

        explicit Connection(DBusConnection * connection);

        void close();

        static v8::Handle<v8::Value> New(v8::Arguments const&);
        static v8::Handle<v8::Value> Send(v8::Arguments const&);
        static v8::Handle<v8::Value> SendWithReply(v8::Arguments const&);
        static v8::Handle<v8::Value> Dispatch(v8::Arguments const&);
        static v8::Handle<v8::Value> Close(v8::Arguments const&);

        static
        v8::Handle<v8::Value>
        GetIsConnected(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        v8::Handle<v8::Value>
        GetIsAuthenticated(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        v8::Handle<v8::Value>
        GetIsAnonymous(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        v8::Handle<v8::Value>
        GetServerId(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        v8::Handle<v8::Value>
        GetDispatchStatus(v8::Local<v8::String> property, const v8::AccessorInfo &info);

        DBusConnection * connection_;
        bool             closed_;
};

} // end of namespace node_dbus

#endif // NODE_DBUS_CONNECTION_INCLUDED
