#include "node_dbus_connection.hpp"

#include <iostream>

#include "v8_utils.hpp"
#include "node_dbus_utils.hpp"
#include "node_dbus_watch.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

//==== dbus watch glue =========================================================
dbus_bool_t 
call_js_watch_function(const char * name, DBusWatch * watch,
        void * connection_data)
{
    HandleScope scope;
    Connection * connection = static_cast<Connection*>(connection_data);
    Local<Value> v = connection->handle_->Get(String::NewSymbol(name));
    if ( ! v->IsFunction()) {
        std::cerr << "ERROR: failed to get addWatch() function" << std::endl;
        return false;
    }
    Local<Function> f = Function::Cast(*v);
    Local<Value> w = Local<Value>::New(static_cast<Watch*>(dbus_watch_get_data(watch))->handle_);
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

//==== Connection ==============================================================

Connection::Connection(DBusConnection * connection) :
      connection_(connection)
    , closed_(false)
{}

Connection::~Connection() {
    std::cout << "Connection::~Connection()" << std::endl;
    if ( ! closed_ ) {
        dbus_connection_close(connection_);
    }
    dbus_connection_unref(connection_);
}

void
Connection::Initialize(v8_utils::ObjectHandle exports) {
    HandleScope scope;
    base::Initialize("Connection", New);

    prototype_method("close", Close);

    exports["Connection"] = function();
}

Handle<Value>
Connection::New(v8::Arguments const& args) {
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

    return args.This();
}

v8::Handle<v8::Value>
Connection::Close(v8::Arguments const& args) {
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

} // end of namespace node_dbus
