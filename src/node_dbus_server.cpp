#include "node_dbus_server.hpp"

#include <dbus/dbus.h>

#include "node_dbus_utils.hpp"
#include "node_dbus_mainloop_callbacks.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

void
Server::Initialize(ObjectHandle exports) {
    HandleScope scope;
    base::Initialize("Server", New);

    exports["Server"] = function();
}

Server::Server(DBusServer * srv) :
    server_(srv)
{}

Server::~Server() {
    if (server_) {
        dbus_server_unref(server_);
    }
}

v8::Handle<v8::Value>
Server::New(v8::Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }

    if ( ! args[0]->IsString()) {
        return throwTypeError("argument 1 must be a string (address)");
    }
    String::Utf8Value address(args[0]->ToString());

    NodeDBusError error;
    DBusServer * srv = dbus_server_listen(*address, error);
    if (error) {
        return error.exception();
    }
    Server * server = new Server(srv);
    server->Wrap(args.This());
    dbus_server_set_watch_functions(
              srv
            , add_watch<Server>
            , remove_watch<Server>
            , toggle_watch<Server>
            , server
            , NULL /*free data*/
    );
    dbus_server_set_timeout_functions(
              srv
            , add_timeout<Server>
            , remove_timeout<Server>
            , toggle_timeout<Server>
            , server
            , NULL /*free data*/
    );
    return args.This();
}

} // end of namespace node_dbus

