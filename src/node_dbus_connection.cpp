#include "node_dbus_connection.hpp"

#include <iostream>

#include "v8_utils.hpp"
#include "node_dbus_utils.hpp"
#include "node_dbus_watch.hpp"
#include "node_dbus_timeout.hpp"
#include "node_dbus_message.hpp"
#include "node_dbus_pending_call.hpp"
#include "node_dbus_mainloop_callbacks.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

static
void
dispatch_status_changed(DBusConnection * connection,
        DBusDispatchStatus new_status, void * data)
{
    std::cerr << "==== dispatch status changed" << std::endl;
    HandleScope scope;
    Connection * c = static_cast<Connection*>(data);
    Local<Value> v = c->handle_->Get(String::NewSymbol("_dispatchStatusChanged"));
    if ( ! v->IsFunction()) {
        std::cerr << "ERROR: failed to get dispatchStatusChanged() function" << std::endl;
        return;
    }
    Local<Function> f = Function::Cast(*v);
    TryCatch trycatch;
    Local<Value> status(Integer::New(new_status));
    f->Call(c->handle_, 1, & status);
    if (trycatch.HasCaught()) {
        Handle<Value> exception = trycatch.Exception();
        String::AsciiValue exception_str(exception);
        std::cerr << *exception_str << std::endl;
    }
}


//==== Connection ==============================================================

Connection::Connection(DBusConnection * connection) :
      connection_(connection)
    , closed_(false)
{}

Connection::~Connection() {
    if ( ! closed_ ) {
        dbus_connection_close(connection_);
    }
    dbus_connection_unref(connection_);
}

void
Connection::Initialize(v8_utils::ObjectHandle exports) {
    HandleScope scope;
    base::Initialize("Connection", New);

    prototype_method("send", Send);
    prototype_method("sendWithReply", SendWithReply);
    prototype_method("dispatch", Dispatch);
    prototype_method("close", Close);

    property("isConnected", GetIsConnected);
    property("isAuthenticated", GetIsAuthenticated);
    property("isAnonymous", GetIsAnonymous);
    property("serverId", GetServerId);
    property("dispatchStatus", GetDispatchStatus);

    exports["Connection"] = function();
}

Handle<Value>
Connection::New(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }
    if ( ! args[0]->IsInt32()) {
        return throwTypeError("argument 1 must be an integer (DBusBusType)");
    }
    DBusBusType type = static_cast<DBusBusType>(args[0]->Int32Value());

    NodeDBusError error;
    DBusConnection * connection = dbus_bus_get_private(type, error);
    if (error) {
        return error.exception();
    }
    Connection * o = new Connection(connection);
    o->Wrap(args.This());
    dbus_connection_set_watch_functions(
              connection
            , add_watch<Connection>
            , remove_watch<Connection>
            , toggle_watch<Connection>
            , o
            , NULL /*free data*/
    );
    dbus_connection_set_timeout_functions(
              connection
            , add_timeout<Connection>
            , remove_timeout<Connection>
            , toggle_timeout<Connection>
            , o
            , NULL /*free data*/
    );
    dbus_connection_set_dispatch_status_function(
              connection
            , dispatch_status_changed
            , o
            , NULL /*free data*/
    );

    return args.This();
}

Handle<Value>
Connection::Send(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1, 3)) {
        return throwArgumentCountMismatchException(args, 1, 3);
    }

    if ( ! args[0]->IsObject()) {
        return throwTypeError("argument 1 must be an object (DBusMessage)");
    }
    Message * msg = Message::unwrap(args[0]->ToObject());
    Connection * c = unwrap(args.This());
    if (args.Length() == 1) {
        dbus_uint32_t serial;
        dbus_bool_t ok = dbus_connection_send(c->connection(), msg->message(),
                &serial);
        if ( ! ok ) {
            return throwError("Out of memory");
        }
        return scope.Close(to_js(serial));
    } else {
        int timeout = 1000;
        int callback_idx = 1;
        if (args.Length() == 3) {
            callback_idx = 2;
            if ( ! args[1]->IsInt32()) {
                return throwTypeError("argument 2 must be an integer (timeout[ms])");
            }
            timeout = args[1]->Int32Value();
        }
        if ( ! args[callback_idx]->IsFunction()) {
            std::ostringstream msg;
            msg << "argument " << callback_idx << " must be a function";
            return throwTypeError(msg.str().c_str());
        }

        DBusPendingCall * pc;
        dbus_bool_t ok = dbus_connection_send_with_reply(c->connection(), msg->message(),
                & pc, timeout);

        if ( ! ok ) {
            return throwError("Out of memory");
        }

        PendingCall * pending_call = PendingCall::New(pc);
        pending_call->setCallback(Local<Function>::Cast(args[callback_idx]));

        return scope.Close(pending_call->handle_);
    }
}

Handle<Value>
Connection::SendWithReply(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }

    if ( ! args[0]->IsObject()) {
        return throwTypeError("argument 1 must be an object (DBusMessage)");
    }
    Message * msg = Message::unwrap(args[0]->ToObject());

    // TODO: Timeout argument
    int timeout = 1000;
    Connection * c = unwrap(args.This());
    
    DBusPendingCall * pending_call;
    dbus_bool_t ok = dbus_connection_send_with_reply(c->connection(), msg->message(),
            & pending_call, timeout);

    if ( ! ok ) {
        return throwError("Out of memory");
    }

    return scope.Close(PendingCall::New(pending_call)->handle_);
}

Handle<Value>
Connection::Dispatch(Arguments const& args) {
    HandleScope scope;
    Connection * c = unwrap(args.This());
    dbus_bool_t ok = dbus_connection_dispatch(c->connection());
    return Undefined();
}

Handle<Value>
Connection::Close(Arguments const& args) {
    HandleScope scope;
    Connection * c = Unwrap<Connection>(args.This());
    c->close();
    return Undefined();
}
void
Connection::close() {
    dbus_connection_close(connection_);
    closed_ = true;
}

Handle<Value>
Connection::GetIsConnected(Local<String> property, const AccessorInfo &info) {
    HandleScope scope;
    Connection * c = unwrap(info.Holder());
    return scope.Close(Boolean::New((dbus_connection_get_is_connected(c->connection()))));
}

Handle<Value>
Connection::GetIsAuthenticated(Local<String> property, const AccessorInfo &info) {
    HandleScope scope;
    Connection * c = unwrap(info.Holder());
    return scope.Close(Boolean::New((dbus_connection_get_is_authenticated(c->connection()))));
}

Handle<Value>
Connection::GetServerId(Local<String> property, const AccessorInfo &info) {
    HandleScope scope;
    Connection * c = unwrap(info.Holder());
    return scope.Close(String::New((dbus_connection_get_server_id(c->connection()))));
}

Handle<Value>
Connection::GetIsAnonymous(Local<String> property, const AccessorInfo &info) {
    HandleScope scope;
    Connection * c = unwrap(info.Holder());
    return scope.Close(Boolean::New((dbus_connection_get_is_anonymous(c->connection()))));
}

Handle<Value>
Connection::GetDispatchStatus(Local<String> property, const AccessorInfo &info) {
    HandleScope scope;
    Connection * c = unwrap(info.Holder());
    return scope.Close(Integer::New((dbus_connection_get_dispatch_status(c->connection()))));
}

} // end of namespace node_dbus
