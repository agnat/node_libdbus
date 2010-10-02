#ifndef NODE_UTILS_WRAPPED_INCLUDED
#define NODE_UTILS_WRAPPED_INCLUDED

#include <sstream>
#include <node.h>

namespace v8_utils {

template <typename T>
struct convert_to_js;

template <>
struct convert_to_js<const char*> {
    static inline v8::Handle<v8::Value> convert(const char * str) {
        return v8::String::New(str);
    }
};

template <>
struct convert_to_js<int> {
    static inline v8::Handle<v8::Value> convert(int i) {
        return v8::Integer::New(i);
    }
};

template <>
struct convert_to_js<unsigned> {
    static inline v8::Handle<v8::Value> convert(unsigned u) {
        return v8::Integer::NewFromUnsigned(u);
    }
};


template <>
struct convert_to_js<bool> {
    static inline v8::Handle<v8::Value> convert(bool b) {
        return v8::Boolean::New(b);
    }
};


template <typename T>
inline
v8::Handle<v8::Value>
to_js(T const & v) {
    return convert_to_js<T>::convert(v);
}

inline
void
defineFunction(v8::Handle<v8::Object> target, const char * name, v8::InvocationCallback f) {
    target->Set(v8::String::NewSymbol(name),
            v8::FunctionTemplate::New(f)->GetFunction());
}

inline
v8::Handle<v8::Value>
throwError(const char * message) {
    return ThrowException( v8::Exception::Error( v8::String::New( message )));
}

inline
v8::Handle<v8::Value>
throwTypeError(const char * message) {
    return ThrowException( v8::Exception::TypeError( v8::String::New( message )));
}

inline
bool
argumentCountMismatch(v8::Arguments const& args, size_t expected) {
    return args.Length() != expected;
}

inline
v8::Handle<v8::Value>
throwArgumentCountMismatchException(v8::Arguments const& args, size_t expectedCount) {
    std::ostringstream msg;
    msg << "argument count mismatch: expected " << expectedCount 
        << ", but got " <<  args.Length() << " arguments.";
    return throwError(msg.str().c_str());
}

template <typename T>
class Wrapped : public node::ObjectWrap {
    public:
        static
        void
        Initialize(const char * className, v8::InvocationCallback ctor = New) {
            v8::HandleScope scope;

            v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(ctor);
            constructor_template = v8::Persistent<v8::FunctionTemplate>::New(t);
            constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
            classname = className;
            v8::Local<v8::String> js_classname(v8::String::NewSymbol(classname));
            constructor_template->SetClassName(js_classname);

            //target->Set(js_classname, constructor_template->GetFunction());
        }

        static
        v8::Local<v8::Function>
        function() {
            return constructor_template->GetFunction();
        }

        static inline
        T *
        unwrap(v8::Handle<v8::Object> obj) {
            return Unwrap<T>(obj);
        }

        static inline
        bool
        HasInstance(v8::Handle<v8::Value> obj) {
            return constructor_template->HasInstance(obj);
        }

        static v8::Persistent<v8::FunctionTemplate> constructor_template;
        static const char * classname;
    protected:
        static
        v8::Handle<v8::Value>
        Unconstructable(v8::Arguments const& args) {
            v8::HandleScope handle;
            std::ostringstream msg;
            msg << "constructor: " << classname << " is not constructable";
            return v8_utils::throwError(msg.str().c_str());
        }

        static
        void
        prototype_method(const char * name, v8::InvocationCallback f) {
            v8::Local<v8::Signature> sig = v8::Signature::New(constructor_template);
            v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(
                  f, v8::Handle<v8::Value>(), sig);
            constructor_template->PrototypeTemplate()->Set(
                    v8::String::NewSymbol(name), t);
        }

        static
        void
        property(const char * name, v8::AccessorGetter getter, v8::AccessorSetter setter = NULL) {
            constructor_template->InstanceTemplate()->SetAccessor(
                  v8::String::NewSymbol(name)
                , getter
                , setter
            );
        }
        
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

template <typename T>
const char *
Wrapped<T>::classname = NULL;

//=============================================================================

struct ObjectHandle;

struct PropertyProxy {
    public:
        PropertyProxy(v8::Handle<v8::Object> * o, const char * name) :
            object_(o), name_(name)
        {}
        ~PropertyProxy();

        void operator=(v8::Local<v8::Function> f) {
            function_ = f;
        }
    private:
        v8::Handle<v8::Object> * object_;
        const char* name_;
        v8::Local<v8::Function> function_;
};

class ObjectHandle : public v8::Handle<v8::Object> {
        typedef v8::Handle<v8::Object> base;
    public:
        ObjectHandle(v8::Handle<v8::Object> o) : base(o) {}

        PropertyProxy operator[](const char * name) {
            return PropertyProxy(this, name);
        }
};

inline
PropertyProxy::~PropertyProxy() {
    if (object_ && name_/* && function_*/) {
        (*object_)->Set(v8::String::NewSymbol(name_), function_);
    }
}
} // end of namespace v8_utils

#endif // NODE_UTILS_WRAPPED_INCLUDED
