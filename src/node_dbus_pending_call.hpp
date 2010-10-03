#ifndef NODE_DBUS_PENDING_CALL_INCLUDED
#define NODE_DBUS_PENDING_CALL_INCLUDED

#include <dbus/dbus.h>

#include "v8_utils.hpp"

namespace node_dbus {

class PendingCall : public v8_utils::Wrapped<PendingCall> {
        typedef v8_utils::Wrapped<PendingCall> base;
    public:
        static void Initialize(v8_utils::ObjectHandle exports);

        static PendingCall * New(DBusPendingCall * pc);

        inline
        DBusPendingCall *
        pending_call() { return pending_call_; }
        ~PendingCall();
    private:
        PendingCall();
        PendingCall(DBusPendingCall * call);

        static v8::Handle<v8::Value> New(v8::Arguments const&);
        static v8::Handle<v8::Value> SetNotifiy(v8::Arguments const&);
        static v8::Handle<v8::Value> Cancel(v8::Arguments const&);

        static
        v8::Handle<v8::Value>
        GetCompleted(v8::Local<v8::String> property, const v8::AccessorInfo &info);

        static void OnResult(DBusPendingCall * pc, void * data);
        
        DBusPendingCall * pending_call_;
        v8::Persistent<v8::Function> callback_;

        static dbus_int32_t data_slot;
};

} // end of namespace node_dbus

#endif // NODE_DBUS_PENDING_CALL_INCLUDED
