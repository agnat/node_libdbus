#ifndef NODE_DBUS_MAINLOOP_CALLBACKS_INCLUDED
#define NODE_DBUS_MAINLOOP_CALLBACKS_INCLUDED

#include "node_dbus_watch.hpp"
#include "node_dbus_timeout.hpp"

namespace node_dbus {

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

template <typename OwnerT, typename WrapperT, typename DBusT>
dbus_bool_t
call_dbus_glue(const char * name, DBusT * d, void * data) {
    v8::HandleScope scope;
    OwnerT * owner = static_cast<OwnerT*>(data);
    v8::Local<v8::Value> v = owner->handle_->Get(v8::String::NewSymbol(name));
    if ( ! v->IsFunction()) {
        std::cerr << "ERROR: failed to get " << name << "() function" << std::endl;
        return false;
    }
    v8::Local<v8::Function> f = v8::Function::Cast(*v);
    v8::Local<v8::Value> w = v8::Local<v8::Value>::New(static_cast<WrapperT*>(detail::get_data(d))->handle_);
    v8::TryCatch trycatch;
    f->Call(owner->handle_, 1, & w);
    if (trycatch.HasCaught()) {
        v8::Handle<v8::Value> exception = trycatch.Exception();
        v8::String::AsciiValue exception_str(exception);
        std::cerr << *exception_str << std::endl;
        return false;
    }
    return true;
}

template <typename OwnerT>
inline
dbus_bool_t 
call_js_watch_function(const char * name, DBusWatch * watch, void * data) {
    return call_dbus_glue<OwnerT, Watch>(name, watch, data);
}

template <typename OwnerT>
inline
dbus_bool_t 
call_js_timeout_function(const char * name, DBusTimeout * t, void * data) {
    return call_dbus_glue<OwnerT, Timeout>(name, t, data);
}

template <typename OwnerT>
inline
dbus_bool_t
add_watch(DBusWatch * watch, void * data) {
    Watch * w = Watch::New(watch);
    dbus_watch_set_data(watch, w, NULL /* free */);
    return call_js_watch_function<OwnerT>("_addWatch", watch, data);
}

template <typename OwnerT>
inline
void
remove_watch(DBusWatch * watch, void * data) {
    call_js_watch_function<OwnerT>("_removeWatch", watch, data);
}

template <typename OwnerT>
inline
void
toggle_watch(DBusWatch * watch, void * data) {
    call_js_watch_function<OwnerT>("_toggleWatch", watch, data);
}

template <typename OwnerT>
inline
dbus_bool_t
add_timeout(DBusTimeout * timeout, void * data) {
    Timeout * t = Timeout::New(timeout);
    dbus_timeout_set_data(timeout, t, NULL /* free */);
    return call_js_timeout_function<OwnerT>("_addTimeout", timeout, data);
}

template <typename OwnerT>
inline
void
remove_timeout(DBusTimeout * timeout, void * data) {
    call_js_timeout_function<OwnerT>("_removeTimeout", timeout, data);
}

template <typename OwnerT>
inline
void
toggle_timeout(DBusTimeout * timeout, void * data) {
    call_js_timeout_function<OwnerT>("_toggleTimeout", timeout, data);
}

} // end of namespace node_dbus
#endif // NODE_DBUS_MAINLOOP_CALLBACKS_INCLUDED
