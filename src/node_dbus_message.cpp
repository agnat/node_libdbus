#include "node_dbus_message.hpp"

#include <iostream>

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

Message::Message(DBusMessage * msg) :
    base(),
    message_(msg)
{
}

void
Message::Initialize(v8_utils::ObjectHandle epxorts) {
    base::Initialize("Message", New);
}

Handle<Value>
Message::New(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }
    Message * msg;
    if ( args[0]->IsExternal()) {
        msg = new Message(static_cast<DBusMessage*>(External::Unwrap(args[0])));
    } else if ( args[0]->IsInt32()) {
        int type = args[0]->Int32Value();
        msg = new Message(dbus_message_new(type));
    } else {
       return throwTypeError("argument 1 must be an integer (DBusMessageType)");
    }

    msg->Wrap(args.This());
    return args.This();
}

Handle<Value>
Message::CreateMethodCall(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 4)) {
        return throwArgumentCountMismatchException(args, 4);
    }

    if ( ! args[0]->IsString()) {
        return throwTypeError("argument 1 must be a string (destination)");
    }
    String::Utf8Value destination(args[0]->ToString());

    if ( ! args[1]->IsString()) {
        return throwTypeError("argument 2 must be a string (path)");
    }
    String::Utf8Value path(args[1]->ToString());

    if ( ! args[2]->IsString()) {
        return throwTypeError("argument 3 must be a string (interface)");
    }
    String::Utf8Value interface(args[2]->ToString());

    if ( ! args[3]->IsString()) {
        return throwTypeError("argument 4 must be a string (method)");
    }
    String::Utf8Value method(args[3]->ToString());

    DBusMessage * msg = dbus_message_new_method_call(
              *destination
            , *path
            , *interface
            , *method
    );
    if ( ! msg) {
        return throwError("Out of memory");
    }
    Local<Value> arg(External::New(msg));
    return scope.Close(Message::function()->NewInstance(1, & arg));
}

Handle<Value>
Message::CreateMethodReturn(Arguments const&) {
}

Handle<Value>
Message::CreateErrorMessage(Arguments const&) {
}

Handle<Value>
Message::CreateSignal(Arguments const&) {
}

} // end of namespace node_dbus
