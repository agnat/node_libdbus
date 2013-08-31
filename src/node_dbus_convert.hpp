#ifndef NODE_DBUS_CONVERTER2_INCLUDED
#define NODE_DBUS_CONVERTER2_INCLUDED

#include <tr1/memory>
#include <vector>
#include <iostream>
#include <v8.h>
#include <dbus/dbus.h>

namespace node_dbus { namespace detail {

enum ContainerType {
    CONTAINER_ARRAY,
    CONTAINER_DICT
};

class v8_iterator {
    public:
        v8_iterator(v8::Handle<v8::Value> src) :
              src_(v8::Persistent<v8::Object>::New(src->ToObject()))
            , idx_()
            , end_()
            , keys_() 
        {
            init();
        }
        v8_iterator(v8_iterator & parent, ContainerType type) :
              src_(v8::Persistent<v8::Object>::New((*parent)->ToObject()))
            , idx_()
            , end_()
            , keys_()
        {
            init();
        }

        void
        next() {
            if (keys_.IsEmpty()) {
                ++idx_;
            } else {
                do {
                    ++idx_;
                } while ( idx_ < end_ &&
                        ! src_->HasRealNamedProperty(keys_->Get(v8::Integer::New(idx_))->ToString()));
            }
        }

        operator bool() const { return idx_ < end_; }

        v8::Handle<v8::Value> operator *() const {
            v8::HandleScope scope;
            if ( keys_.IsEmpty()) {
                return scope.Close(src_->Get(idx_));
            } else {
                v8::Local<v8::Value> key = keys_->Get(idx_);
                return scope.Close(src_->Get(key));
            };
        }
    private:

        void
        init() {
            v8::HandleScope scope;
            if (src_->IsArray()) {
                end_ = v8::Array::Cast(*src_)->Length();
            } else {
                keys_ = v8::Persistent<v8::Array>::New(src_->ToObject()->GetPropertyNames());
                end_ = keys_->Length();
            }
        }

        v8::Persistent<v8::Object> src_;
        size_t                     idx_;
        size_t                     end_;
        v8::Persistent<v8::Array>  keys_;
};

struct dbus_iterator {
    dbus_iterator(DBusMessage * src) {
        dbus_message_iter_init( src, & it_ );
        cache_state();
    }
    dbus_iterator(dbus_iterator & parent, ContainerType type) {
        dbus_message_iter_recurse( & parent.it_, & it_ );
        cache_state();
    }
    void next() {
        dbus_message_iter_next( & it_ );
        cache_state();
    }

    operator bool() const { return type_ != DBUS_TYPE_INVALID; }

    int current_type() const { return type_; }
    int current_element_type() const { return element_type_; }

    template <typename T>
    T
    get() {
        T v;
        dbus_message_iter_get_basic(& it_, & v);
        return v;
    }
    inline
    void
    cache_state() {
        type_ = dbus_message_iter_get_arg_type( & it_ );
        element_type_ = (type_ == DBUS_TYPE_ARRAY
                ? dbus_message_iter_get_element_type( & it_ )
                : DBUS_TYPE_INVALID);
    }

    DBusMessageIter it_;
    int type_;
    int element_type_;
};

struct v8_sink {
    v8_sink(v8::Handle<v8::Value> dst) :
          dst_(v8::Persistent<v8::Object>::New(dst->ToObject()))
        , idx_()
    {}
    v8_sink(v8_sink & parent, ContainerType type) :
        idx_()
    {
        if (type == CONTAINER_ARRAY) {
            dst_ = v8::Persistent<v8::Array>::New(v8::Array::New());
            parent.add(dst_);
        } else {
            std::cerr << "v8_sink: TODO handle dict" << std::endl;
        }
    }
    ~v8_sink() {
        if ( ! dst_.IsEmpty()) dst_.Dispose();
    }
    template <typename S>
    void accept(dbus_iterator /*const*/& it, S & stack) {
        switch (it.current_type()) {
        case DBUS_TYPE_BOOLEAN:
            dst_->Set(idx_++, v8::Boolean::New(it.get<bool>()));
            break;
        case DBUS_TYPE_DOUBLE:
            dst_->Set(idx_++, v8::Number::New(it.get<double>()));
            break;
        case DBUS_TYPE_INT32:
            dst_->Set(idx_++, v8::Integer::New(it.get<int32_t>()));
            break;
        case DBUS_TYPE_UINT32:
            dst_->Set(idx_++, v8::Integer::NewFromUnsigned(it.get<uint32_t>()));
            break;
        case DBUS_TYPE_STRING:
            dst_->Set(idx_++, v8::String::New(it.get<const char *>()));
            break;
        case DBUS_TYPE_ARRAY:
        case DBUS_TYPE_STRUCT:
            typedef typename S::value_type::element_type frame;
            typedef typename S::value_type frame_ptr;
            stack.push_back(frame_ptr(new frame(it, *this, CONTAINER_ARRAY)));
            break;
        default:
            std::cerr << "v8 sink: unhandled type" << std::endl;
            break;
        }
    }

    void
    add(v8::Handle<v8::Object> obj) {
        dst_->Set(idx_, obj);
    }
    v8::Persistent<v8::Object> dst_;
    size_t                     idx_; // TODO: key value stuff
};

class dbus_sink {
    public:
        dbus_sink(DBusMessage * dst) : requires_close_(false) {
            dbus_message_iter_init_append(dst, & it_);
        }
        dbus_sink(dbus_sink & parent, ContainerType type) :
              requires_close_(true)
            , parent_( & parent.it_ )
        {
            dbus_message_iter_open_container( & parent.it_, DBUS_TYPE_STRUCT, NULL, & it_);
        }
        ~dbus_sink() {
            if (requires_close_) {
                dbus_message_iter_close_container( parent_, & it_);
            }
        }

        template <typename S>
        void accept(v8_iterator & it, S & stack) {
            if ((*it)->IsBoolean()) {
                bool v = (*it)->ToBoolean()->Value();
                dbus_message_iter_append_basic( & it_, DBUS_TYPE_BOOLEAN, & v);
            } else if ((*it)->IsInt32()) {
                int32_t v = (*it)->Int32Value();
                dbus_message_iter_append_basic( & it_, DBUS_TYPE_INT32, & v);
            } else if ((*it)->IsUint32()) {
                uint32_t v = (*it)->Uint32Value();
                dbus_message_iter_append_basic( & it_, DBUS_TYPE_UINT32, & v);
            } else if ((*it)->IsNumber()) {
                double v = (*it)->NumberValue();
                dbus_message_iter_append_basic( & it_, DBUS_TYPE_DOUBLE, & v);
            } else if ((*it)->IsString()) {
                v8::String::Utf8Value v((*it)->ToString());
                const char * str = *v;
                dbus_message_iter_append_basic( & it_, DBUS_TYPE_STRING, & str);
            } else if ((*it)->IsArray()) {
                typedef typename S::value_type::element_type frame;
                typedef typename S::value_type frame_ptr;
                stack.push_back(frame_ptr(new frame(it, *this, CONTAINER_ARRAY)));
            } else {
                std::cerr << "dbus sink: unhandled type" << std::endl;
            }
        }
    private:
        dbus_sink();
        dbus_sink(dbus_sink const&);
        dbus_sink & operator=(dbus_sink const&);
        DBusMessageIter it_;
        bool requires_close_;
        DBusMessageIter * parent_;
};

template <typename SrcT> struct select_iterator { typedef v8_iterator type; };
template <> struct select_iterator<DBusMessage*> { typedef dbus_iterator type; };

template <typename DstT> struct select_sink { typedef v8_sink type; };
template <> struct select_sink<DBusMessage*> { typedef dbus_sink type; };


template <typename SrcT, typename DstT>
struct stack_frame {
    typedef typename select_iterator<SrcT>::type iterator;
    typedef typename select_sink<DstT>::type     sink;

    stack_frame(SrcT src, DstT dst) : it_(src), sink_(dst) {}

    stack_frame(iterator & parent_it, sink & parent_sink, ContainerType type) :
          it_(parent_it, type)
        , sink_(parent_sink, type)
    {}

    template <typename S>
    bool
    work(S & stack) {
        if ( ! it_) {
            return false;
        }
        sink_.accept(it_, stack);
        it_.next();
        return true;
    }

    iterator it_;
    sink     sink_;
};

} // end of namespace detail

template <typename SrcT, typename DstT>
void
convert(SrcT src, DstT dst, const char * signature = NULL) {
    using detail::stack_frame;
    typedef stack_frame<SrcT, DstT> frame;
    typedef std::tr1::shared_ptr<frame> frame_ptr;
    typedef std::vector<frame_ptr> stack;
    stack s;
    s.push_back(frame_ptr( new frame(src, dst)));
    while ( ! s.empty() ) {
        if ( ! s.back()->work(s)) {
            s.pop_back();
        }
    }
}

} // end of namespace node_dbus
#endif // NODE_DBUS_CONVERTER2_INCLUDED
