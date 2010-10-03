#include "node_dbus_connection.hpp"

#include <iostream>

#include "v8_utils.hpp"
#include "node_dbus_utils.hpp"
#include "node_dbus_watch.hpp"
#include "node_dbus_timeout.hpp"
#include "node_dbus_message.hpp"
#include "node_dbus_pending_call.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

//==== node <-> dbus glue =========================================================

namespace detail {

template <typename DBusT> struct data_accessor;

template <>
struct data_accessor<DBusWatch> {
    static inline
    void *
    get(DBusWatch * watch) { return dbus_watch_get_data(watch); }
};

template <>
struct data_accessor<DBusTimeout> {
    static inline
    void *
    get(DBusTimeout * t) { return dbus_timeout_get_data(t); }
};

template <typename T>
inline
void *
get_data(T * t) {
    return data_accessor<T>::get(t);
}

} // end of namespace detail

template <typename WrapperT, typename DBusT>
dbus_bool_t
call_dbus_glue(const char * name, DBusT * d, void * data) {
    HandleScope scope;
    Connection * connection = static_cast<Connection*>(data);
    Local<Value> v = connection->handle_->Get(String::NewSymbol(name));
    if ( ! v->IsFunction()) {
        std::cerr << "ERROR: failed to get " << name << "() function" << std::endl;
        return false;
    }
    Local<Function> f = Function::Cast(*v);
    Local<Value> w = Local<Value>::New(static_cast<WrapperT*>(detail::get_data(d))->handle_);
    TryCatch trycatch;
    f->Call(connection->handle_, 1, & w);
    if (trycatch.HasCaught()) {
        Handle<Value> exception = trycatch.Exception();
        String::AsciiValue exception_str(exception);
        std::cerr << *exception_str << std::endl;
        return false;
    }
    return true;
}

inline
dbus_bool_t 
call_js_watch_function(const char * name, DBusWatch * watch,
        void * data)
{
    return call_dbus_glue<Watch>(name, watch, data);
}

inline
dbus_bool_t 
call_js_timeout_function(const char * name, DBusTimeout * t,
        void * data)
{
    return call_dbus_glue<Timeout>(name, t, data);
}

static
dbus_bool_t
add_watch(DBusWatch * watch, void * data) {
    Watch * w = Watch::New(watch);
    dbus_watch_set_data(watch, w, NULL /* free */);
    return call_js_watch_function("addWatch", watch, data);
}

static
void
remove_watch(DBusWatch * watch, void * data) {
    call_js_watch_function("removeWatch", watch, data);
}

static
void
toggle_watch(DBusWatch * watch, void * data) {
    call_js_watch_function("toggleWatch", watch, data);
}

static
dbus_bool_t
add_timeout(DBusTimeout * timeout, void * data) {
    Timeout * t = Timeout::New(timeout);
    dbus_timeout_set_data(timeout, t, NULL /* free */);
    return call_js_timeout_function("addTimeout", timeout, data);
}

static
void
remove_timeout(DBusTimeout * timeout, void * data) {
    call_js_timeout_function("removeTimeout", timeout, data);
}

static
void
toggle_timeout(DBusTimeout * timeout, void * data) {
    call_js_timeout_function("toggleTimeout", timeout, data);
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
            , add_watch
            , remove_watch
            , toggle_watch
            , o
            , NULL /*free data*/
    );
    dbus_connection_set_timeout_functions(
              connection
            , add_timeout
            , remove_timeout
            , toggle_timeout
            , o
            , NULL /*free data*/
    );

    return args.This();
}

Handle<Value>
Connection::Send(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }

    if ( ! args[0]->IsObject()) {
        return throwTypeError("argument 1 must be an object (DBusMessage)");
    }
    Message * msg = Message::unwrap(args[0]->ToObject());
    Connection * c = unwrap(args.This());
    dbus_uint32_t serial;
    dbus_bool_t ok = dbus_connection_send(c->connection(), msg->message(),
            &serial);
    if ( ! ok ) {
        return throwError("Out of memory");
    }

    return scope.Close(to_js(serial));
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
