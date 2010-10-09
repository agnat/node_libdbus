#ifndef NODE_DBUS_SERVER_INCLUDED
#define NODE_DBUS_SERVER_INCLUDED

#include "v8_utils.hpp"

struct DBusServer;

namespace node_dbus {

class Server : public v8_utils::Wrapped<Server> {
        typedef v8_utils::Wrapped<Server> base;
    public:
        static void Initialize(v8_utils::ObjectHandle exports);

        inline DBusServer* server() { return server_; }

        ~Server();
    private:    
        Server();
        Server(Server const&);
        Server const& operator=(Server const&);

        explicit Server(DBusServer * srv);

        static v8::Handle<v8::Value> New(v8::Arguments const&);

        DBusServer * server_;
};

} // end of namespace node_dbus
#endif // NODE_DBUS_SERVER_INCLUDED
