#ifndef NODE_DBUS_MESSAGE_ITER_INCLUDED
#define NODE_DBUS_MESSAGE_ITER_INCLUDED

#include <dbus/dbus.h>

#include "v8_utils.hpp"

namespace node_dbus {

class MessageIter : public v8_utils::Wrapped<MessageIter> {
        typedef v8_utils::Wrapped<MessageIter> base;
    public:
        static void Initialize(v8_utils::ObjectHandle exports);

        ~MessageIter();

        inline DBusMessageIter * iter() { return & it_; }

    private:
        MessageIter();
        static inline MessageIter * New() {
            return unwrap( function()->NewInstance(0, NULL) );
        }

        MessageIter(MessageIter const&);
        MessageIter const& operator=(MessageIter const&);

        explicit MessageIter(DBusMessageIter * it);

        static v8::Handle<v8::Value> New(v8::Arguments const&);
        static v8::Handle<v8::Value> HasNext(v8::Arguments const&);
        static v8::Handle<v8::Value> Next(v8::Arguments const&);
        static v8::Handle<v8::Value> GetArgType(v8::Arguments const&);
        static v8::Handle<v8::Value> GetElementType(v8::Arguments const&);
        static v8::Handle<v8::Value> Recurse(v8::Arguments const&);
        static v8::Handle<v8::Value> GetSignature(v8::Arguments const&);
        static v8::Handle<v8::Value> GetBasic(v8::Arguments const&);
        static v8::Handle<v8::Value> GetFixedArray(v8::Arguments const&);
        static v8::Handle<v8::Value> AppendBasic(v8::Arguments const&);

        DBusMessageIter it_;
};

} // end of namespace node_dbus
#endif // NODE_DBUS_MESSAGE_ITER_INCLUDED
