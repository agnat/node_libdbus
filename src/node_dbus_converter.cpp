#include "node_dbus_converter.hpp"

#include <iostream>

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

struct InvalidTypeException {};

Local<Value> convertArray(InserterStack & stack, DBusMessageIter * it, bool * new_container);
Local<Value> convertString(DBusMessageIter * it);
Local<Value> convertBool(DBusMessageIter * it);

class Inserter {
    public:
        virtual void doWork(InserterStack & stack) = 0;
        inline DBusMessageIter * it() { return & it_; }
        bool hasNext() { return dbus_message_iter_has_next(it()); }
        void next() { dbus_message_iter_next(it()); }
    protected:
        Inserter(Handle<Object> obj) : obj_(Persistent<Object>::New(obj)) {}
        Handle<Value> convert(InserterStack & stack);

        Persistent<Object> obj_;
    private:
        Inserter();
        DBusMessageIter    it_;

};

Handle<Value>
Inserter::convert(InserterStack & stack) {
    HandleScope scope;
    Local<Value> v = Local<Value>::New(Undefined());
    bool new_container = false;
    int type = dbus_message_iter_get_arg_type(it());
    switch (type) {
        case DBUS_TYPE_INVALID:
            stack.pop_back();
            if ( ! stack.empty()) {
                dbus_message_iter_next(stack.back()->it());
            }
            return Undefined();
        case DBUS_TYPE_BYTE:
            break;
        case DBUS_TYPE_BOOLEAN:
            v = convertBool(it());
            break;
        case DBUS_TYPE_INT16:
            break;
        case DBUS_TYPE_UINT16:
            break;
        case DBUS_TYPE_INT32:
            break;
        case DBUS_TYPE_UINT32:
            break;
        case DBUS_TYPE_INT64:
            break;
        case DBUS_TYPE_UINT64:
            break;
        case DBUS_TYPE_DOUBLE:
            break;
        case DBUS_TYPE_STRING:
            v = convertString(it());
            break;
        case DBUS_TYPE_OBJECT_PATH:
            break;
        case DBUS_TYPE_SIGNATURE:
            break;
# if defined(DBUS_TYPE_UNIX_FD)
        case DBUS_TYPE_UNIX_FD:
            break;
# endif
        case DBUS_TYPE_ARRAY:
            v = convertArray(stack, it(), & new_container);
            break;
        case DBUS_TYPE_VARIANT:
            break;
        case DBUS_TYPE_STRUCT:
            break;
        case DBUS_TYPE_DICT_ENTRY:
            break;

        default:
            throw InvalidTypeException();
            break;
    }
    if ( ! new_container) {
        dbus_message_iter_next(it());
    }
    return scope.Close(v);
}

class ArrayInserter : public Inserter {
    public:
        ArrayInserter(Handle<Object> array) :
              Inserter(array)
            , idx_()
        {}
        virtual void doWork(InserterStack & stack) {
            HandleScope scope;
            Local<Value> v = Local<Value>::New(convert(stack));
            if ( ! v->IsUndefined()) {
                obj_->Set(idx_++, v);
            }
        }
    private:
        size_t idx_;
};

Local<Value>
convertArray(InserterStack & stack, DBusMessageIter * it, bool * new_container) {
    * new_container = false;
    int element_type = dbus_message_iter_get_element_type(it);
    Local<Value> a = Local<Value>::New(Array::New());
    if ( ! dbus_type_is_fixed(element_type)) {
        * new_container = true;
        stack.push_back(InserterPtr( new ArrayInserter(a->ToObject()) ));
        dbus_message_iter_recurse( it, stack.back()->it());
    }
    return a;
}

Local<Value>
convertString(DBusMessageIter * it) {
    char * result;
    dbus_message_iter_get_basic(it, & result);
    return String::New(result);
}

Local<Value>
convertBool(DBusMessageIter * it) {
    bool result;
    dbus_message_iter_get_basic(it, & result);
    return Local<Value>::New(Boolean::New(result));
}

Handle<Value>
Converter::convert(Message * msg) {
    HandleScope scope;
    Local<Array> a(Array::New());
    stack.push_back(InserterPtr( new ArrayInserter(a)));
    dbus_message_iter_init(msg->message(), stack.back()->it());
    while ( ! stack.empty()) {
        stack.back()->doWork(stack);
    }

    return scope.Close(a);
}

} // end of namespace node_dbus

