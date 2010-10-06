#ifndef NODE_DBUS_CONVERTER2_INCLUDED
#define NODE_DBUS_CONVERTER2_INCLUDED

#include <vector>
#include <iostream>
#include <v8.h>
#include <dbus/dbus.h>

namespace node_dbus { namespace detail {

struct V8_tag {};
struct DBus_tag {};

template <typename Tag> struct other_tag;
template <> struct other_tag<V8_tag>   { typedef DBus_tag type; };
template <> struct other_tag<DBus_tag> { typedef V8_tag   type;  };

template <typename Tag> struct result;
template <> struct result<V8_tag>   { typedef v8::Handle<v8::Value> type; };
template <> struct result<DBus_tag> { typedef DBusMessage *         type; };

template <typename Tag> struct input;
template <> struct input<V8_tag>   { typedef v8::Handle<v8::Value> type; };
template <> struct input<DBus_tag> { typedef DBusMessage *         type; };

template <typename Tag> struct iterator;

struct v8_iterator {
    v8_iterator(v8::Handle<v8::Value> src) :
          src_(v8::Persistent<v8::Object>::New(src->ToObject()))
        , idx_()
        , end_()
        , keys_() 
    {
        v8::HandleScope scope;
        if (src->IsArray()) {
            std::cerr << "iterator ARRAY" << std::endl;
            end_ = v8::Array::Cast(*src)->Length();
        } else {
            keys_ = v8::Persistent<v8::Array>::New(src->ToObject()->GetPropertyNames());
            end_ = keys_->Length();
        }
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
          dst_(dst->ToObject())
        , idx_()
    {}
    template <typename S>
    void accept(dbus_iterator /*const*/& it, S & stack) {
        if (it.current_type() == DBUS_TYPE_BOOLEAN) {
        } else if (it.current_type() == DBUS_TYPE_INT32) {
            dst_->Set(idx_++, v8::Integer::New(it.get<int32_t>()));
        }
    }
    v8::Handle<v8::Object> dst_;
    size_t                 idx_; // TODO: key value stuff
};

struct dbus_sink {
    dbus_sink(DBusMessage * dst) {
        dbus_message_iter_init_append(dst, & it_);
    }
    template <typename S>
    void accept(v8_iterator const& it, S & stack) {
        if ((*it)->IsBoolean()) {
            bool v = (*it)->ToBoolean()->Value();
            dbus_message_iter_append_basic( & it_, DBUS_TYPE_BOOLEAN, & v);
        } else if ((*it)->IsInt32()) {
            int32_t v = (*it)->Int32Value();
            dbus_message_iter_append_basic( & it_, DBUS_TYPE_INT32, & v);
        } else if ((*it)->IsUint32()) {
            uint32_t v = (*it)->Uint32Value();
            dbus_message_iter_append_basic( & it_, DBUS_TYPE_UINT32, & v);
        } else {
            std::cerr << "unhandled type" << std::endl;
        }
    }
    DBusMessageIter it_;
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

/*
template <typename TargetTag>
struct convert_to {
    typedef TargetTag                            target_tag;
    typedef typename other_tag<target_tag>::type source_tag;
    typedef stack_frame<source_tag, target_tag>  stack_frame_type;
    typedef std::vector<stack_frame_type>        stack_type;
    typedef typename result<target_tag>::type    result_type;
    typedef typename input<source_tag>::type     input_type;

    static
    void
    convert(input_type src, result_type dst) {
        stack_type stack;
        stack.push_back(stack_frame_type(src, dst));
        while ( ! stack.empty() ) {
            if ( ! stack.back().work(stack)) {
                stack.pop_back();
            }
        }
    }
};
*/
template <typename SrcT, typename DstT>
void
convert(SrcT src, DstT dst) {
    typedef stack_frame<SrcT, DstT> frame;
    typedef std::vector<frame> stack;
    stack s;
    s.push_back(frame(src, dst));
    while ( ! s.empty() ) {
        if ( ! s.back().work(s)) {
            s.pop_back();
        }
    }
}


}} // end of namespace detail, node_dbus
#endif // NODE_DBUS_CONVERTER2_INCLUDED
