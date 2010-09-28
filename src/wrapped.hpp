#ifndef NODE_UTILS_WRAPPED_INCLUDED
#define NODE_UTILS_WRAPPED_INCLUDED

#include <node.h>

namespace node_utils {

template <typename T>
class Wrapped : public node::ObjectWrap {
    public:
        static
        void
        Initialize(v8::Handle<v8::Object> target, const char * classname,
                v8::InvocationCallback ctor = New)
        {
            v8::HandleScope scope;

            v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(ctor);
            constructor_template = v8::Persistent<v8::FunctionTemplate>::New(t);
            constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
            v8::Local<v8::String> js_classname(v8::String::NewSymbol(classname));
            constructor_template->SetClassName(js_classname);

            target->Set(js_classname, constructor_template->GetFunction());
        }

    protected:
        static v8::Persistent<v8::FunctionTemplate> constructor_template;
        
    private:
        static
        v8::Handle<v8::Value>
        New(v8::Arguments const & args) {
            v8::HandleScope scope;
            T * o = new T();
            o->Wrap(args.This());
            return args.This();
        }
};

template <typename T>
v8::Persistent<v8::FunctionTemplate>
Wrapped<T>::constructor_template = v8::Persistent<v8::FunctionTemplate>();

} // end of namespace node_utils

#endif // NODE_UTILS_WRAPPED_INCLUDED
