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
        Message(Message const&);
        Message const& operator=(Message const&);

        explicit Message(DBusMessage * msg);

        void appendArgs(v8::Arguments const& args, size_t firstArg = 0);

        static v8::Handle<v8::Value> New(v8::Arguments const&);
        static v8::Handle<v8::Value> HasPath(v8::Arguments const&);
        static v8::Handle<v8::Value> HasInterface(v8::Arguments const&);
        static v8::Handle<v8::Value> Args(v8::Arguments const&);
        static v8::Handle<v8::Value> AppendArgs(v8::Arguments const&);

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

        static
        v8::Handle<v8::Value>
        GetType(v8::Local<v8::String> property, const v8::AccessorInfo &info);

        static
        v8::Handle<v8::Value>
        GetNoReply(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        void
        SetNoReply(v8::Local<v8::String> property, v8::Local<v8::Value> value,
                const v8::AccessorInfo& info);

        static
        v8::Handle<v8::Value>
        GetAutoStart(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        void
        SetAutoStart(v8::Local<v8::String> property, v8::Local<v8::Value> value,
                const v8::AccessorInfo& info);

        static
        v8::Handle<v8::Value>
        GetPath(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        void
        SetPath(v8::Local<v8::String> property, v8::Local<v8::Value> value,
                const v8::AccessorInfo& info);

        static
        v8::Handle<v8::Value>
        GetInterface(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        void
        SetInterface(v8::Local<v8::String> property, v8::Local<v8::Value> value,
                const v8::AccessorInfo& info);

        static
        v8::Handle<v8::Value>
        GetMember(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        void
        SetMember(v8::Local<v8::String> property, v8::Local<v8::Value> value,
                const v8::AccessorInfo& info);

        static
        v8::Handle<v8::Value>
        GetDestination(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        void
        SetDestination(v8::Local<v8::String> property, v8::Local<v8::Value> value,
                const v8::AccessorInfo& info);

        static
        v8::Handle<v8::Value>
        GetSender(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        static
        void
        SetSender(v8::Local<v8::String> property, v8::Local<v8::Value> value,
                const v8::AccessorInfo& info);

        static
        v8::Handle<v8::Value>
        GetSignature(v8::Local<v8::String> property, const v8::AccessorInfo &info);
        DBusMessage * message_;
};

} // end of namespace node_dbus
#endif // NODE_DBUS_MESSAGE_INCLUDED
