#ifndef NODE_DBUS_MESSAGE_INCLUDED
#define NODE_DBUS_MESSAGE_INCLUDED

#include "v8_utils.hpp"
#include <dbus/dbus.h>

namespace node_dbus {

class Message : public v8_utils::Wrapped<Message> {
        typedef v8_utils::Wrapped<Message> base;
    public:
        static void Initialize(v8_utils::ObjectHandle epxorts);

        static v8::Handle<v8::Value> CreateMethodCall(v8::Arguments const&);
        static v8::Handle<v8::Value> CreateMethodReturn(v8::Arguments const&);
        static v8::Handle<v8::Value> CreateErrorMessage(v8::Arguments const&);
        static v8::Handle<v8::Value> CreateSignal(v8::Arguments const&);

        static Message * New(DBusMessage * msg);

        inline DBusMessage * message() { return message_; }

        ~Message();
    private:
        Message();
        Message(DBusMessage * msg);

        static v8::Handle<v8::Value> New(v8::Arguments const&);

        static
        v8::Handle<v8::Value>
        GetSerial(v8::Local<v8::String> property, const v8::AccessorInfo &info);

        static
        v8::Handle<v8::Value>
        GetReplySerial(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        void
        SetReplySerial(v8::Local<v8::String> property, v8::Local<v8::Value> value,
                const v8::AccessorInfo& info);

        DBusMessage * message_;
};

} // end of namespace node_dbus
#endif // NODE_DBUS_MESSAGE_INCLUDED
