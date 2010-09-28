#ifndef NODE_DBUS_CONNECTION_INCLUDED
#define NODE_DBUS_CONNECTION_INCLUDED

#include "wrapped.hpp"

namespace node_dbus {

class Connection : public node_utils::Wrapped<Connection> {
        typedef node_utils::Wrapped<Connection> base;
    public:
        static void Initialize(v8::Handle<v8::Object> exports);
    private:
        static v8::Handle<v8::Value> New(v8::Arguments const& args);
};

} // end of namespace node_dbus

#endif // NODE_DBUS_CONNECTION_INCLUDED
