#include "node_dbus_message_iter.hpp"

#include "node_dbus_message.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

MessageIter::MessageIter() : base(), it_() {}

MessageIter::~MessageIter() {}

void
MessageIter::Initialize(v8_utils::ObjectHandle exports) {
    HandleScope scope;
    base::Initialize("MessageIter", New);

    prototype_method("hasNext", HasNext);
    prototype_method("next", Next);
    prototype_method("getArgType", GetArgType);
    prototype_method("getElementType", GetElementType);
    prototype_method("recurse", Recurse);
    prototype_method("getSignature", GetSignature);
    prototype_method("getBasic", GetBasic);
    prototype_method("getFixedArray", GetFixedArray);

    exports["MessageIter"] = function();
}

Handle<Value>
MessageIter::New(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1, 2)) {
        return throwArgumentCountMismatchException(args, 1, 2);
    }
    
    Message * msg = NULL;
    if (args.Length() > 0) {
        if ( ! Message::HasInstance(args[0])) {
            return throwTypeError("argument 1 must be a dbus message");
        }
        msg = Message::unwrap(args[0]->ToObject());
    }

    bool append = false;
    if (args.Length() == 2) {
        if (args[1]->IsBoolean()) {
            append = args[1]->ToBoolean()->Value();
        }
    }
    MessageIter * it = new MessageIter();
    if (args.Length() > 0) {
        if (append) {
            dbus_message_iter_init_append(msg->message(), it->iter());
        } else {
            dbus_message_iter_init(msg->message(), it->iter());
        }
    }

    it->Wrap(args.This());

    return args.This();
}

Handle<Value>
MessageIter::HasNext(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 0)) {
        return throwArgumentCountMismatchException(args, 0);
    }
    MessageIter * it = unwrap(args.This());
    return scope.Close(Boolean::New(dbus_message_iter_has_next(it->iter())));
}

Handle<Value>
MessageIter::Next(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 0)) {
        return throwArgumentCountMismatchException(args, 0);
    }
    MessageIter * it = unwrap(args.This());
    dbus_message_iter_next(it->iter());
    return Undefined();
}

Handle<Value>
MessageIter::GetArgType(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 0)) {
        return throwArgumentCountMismatchException(args, 0);
    }
    MessageIter * it = unwrap(args.This());
    return scope.Close(Integer::New(dbus_message_iter_get_arg_type(it->iter())));
}

Handle<Value>
MessageIter::GetElementType(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 0)) {
        return throwArgumentCountMismatchException(args, 0);
    }
    MessageIter * it = unwrap(args.This());
    return scope.Close(Integer::New(dbus_message_iter_get_element_type(it->iter())));
}

Handle<Value>
MessageIter::Recurse(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 0)) {
        return throwArgumentCountMismatchException(args, 0);
    }
    MessageIter * it = unwrap(args.This());
    MessageIter * sub = New();
    dbus_message_iter_recurse(it->iter(), sub->iter());

    return scope.Close(sub->handle_);
}

Handle<Value>
MessageIter::GetSignature(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 0)) {
        return throwArgumentCountMismatchException(args, 0);
    }
    MessageIter * it = unwrap(args.This());
    char * sig = dbus_message_iter_get_signature(it->iter());
    Local<String> signature(String::New(sig));
    dbus_free(sig);
    return scope.Close(signature);
}

template <typename T>
T
get_basic(MessageIter * it) {
    T v;
    dbus_message_iter_get_basic(it->iter(), & v);
    return v;
}

Handle<Value>
MessageIter::GetBasic(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 0)) {
        return throwArgumentCountMismatchException(args, 0);
    }
    MessageIter * it = unwrap(args.This());
    int type = dbus_message_iter_get_arg_type(it->iter());
    switch (type) {
        case DBUS_TYPE_BOOLEAN:
            return scope.Close(Boolean::New(get_basic<bool>(it)));
        case DBUS_TYPE_INT32:
            return scope.Close(Integer::New(get_basic<int32_t>(it)));
        case DBUS_TYPE_UINT32:
            return scope.Close(Integer::NewFromUnsigned(
                        get_basic<uint32_t>(it)));
        case DBUS_TYPE_STRING:
            return scope.Close(String::New(get_basic<const char*>(it)));

        case DBUS_TYPE_ARRAY:
        case DBUS_TYPE_STRUCT:
            return throwError("iterator does not point to a basic type");

        default:
            return throwError("Unhandled type");
    }
}

Handle<Value>
MessageIter::GetFixedArray(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 0)) {
        return throwArgumentCountMismatchException(args, 0);
    }

    return throwError("not implemented");
}

Handle<Value>
MessageIter::AppendBasic(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 2)) {
        return throwArgumentCountMismatchException(args, 2);
    }
    
    return throwError("not implemented");
}

} // end of namespace node_dbus

