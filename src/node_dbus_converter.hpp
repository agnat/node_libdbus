#ifndef NODE_DBUS_CONVERTER_INCLUDED
#define NODE_DBUS_CONVERTER_INCLUDED

#include <iostream>
#include <vector>
#include <tr1/memory>

#include <node.h>
#include <dbus/dbus.h>

#include "node_dbus_message.hpp"

namespace node_dbus {

class Inserter;
class ArrayInserter;

typedef std::tr1::shared_ptr<Inserter> InserterPtr;

typedef std::vector<InserterPtr> InserterStack;


class Converter {
    public:
       Converter() {};

        v8::Handle<v8::Value> convert(Message * msg);
    private:
        InserterStack stack;
};

class ArgumentAppender {
    public:
        ArgumentAppender(Message * msg) {}
        void append(v8::Handle<v8::Array> args) {
        }
};

//=============================================================================

struct DBusIterator;

enum ContainerType {
    CONTAINER_NONE = 0,
    CONTAINER_DICT,
    CONTAINER_ARRAY
};

struct V8ObjectIterator {
    typedef v8::Handle<v8::Value> ctor_arg_type;

    enum IteratorMode {
          MODE_ARRAY
        , MODE_OBJECT
    };
    V8ObjectIterator(v8::Handle<v8::Value> o, bool isSource) :
        index_()
    {
        if ( ! o->IsObject()) {
            std::cerr << "kaputt" << std::endl;
        }
        if (o->IsArray()) {
            end_ = v8::Array::Cast(*o)->Length();
            mode_ = MODE_ARRAY;
        } else {
            keys_ = v8::Persistent<v8::Array>::New(o->ToObject()->GetPropertyNames());
            end_ = keys_->Length();
            mode_ = MODE_OBJECT;
        }
    }

    template <typename S>
    void
    accept(DBusIterator & src, S & stack) {
    }

    operator bool() const { return index_ < end_; }
    void next() { ++index_; }

    v8::Persistent<v8::Array> keys_;
    size_t index_;
    size_t end_;
    IteratorMode mode_;
};

struct DBusIterator {
    typedef DBusMessage * ctor_arg_type;
    DBusIterator(DBusMessage * msg, bool isSource) :
        isSource_(isSource)
    {
        if (isSource) {
            dbus_message_iter_init(msg, & it);
        } else {
            dbus_message_iter_init_append( msg, & it);
        }
    }
    DBusIterator(DBusIterator * parent) {}

    template <typename S>
    void
    accept(V8ObjectIterator & src, S & stack) {
        std::cerr << "accept dbus" << std::endl;
    }

    operator bool() {
        return dbus_message_iter_get_arg_type( & it) != DBUS_TYPE_INVALID;
    }
    void next() { isSource_ && dbus_message_iter_next( & it); }

    DBusMessageIter it;
    bool isSource_;
};

enum StackOp {
    STACK_OP_POP,
    STACK_OP_PUSH,
    STACK_OP_NONE
};

template <typename SrcIt, typename DstIt>
struct F {
    F(typename SrcIt::ctor_arg_type src, typename DstIt::ctor_arg_type dst) :
          srcIt(src, true)
        , dstIt(dst, false)
    {}

    F(F const& parent, ContainerType type) : srcIt(parent.srcIt), dstIt(parent.dstIt) {}

    template <typename S>
    bool
    work(S & stack) {
        dstIt.accept(srcIt, stack);
        srcIt.next(); dstIt.next();
        return srcIt;
    }

    SrcIt srcIt;
    DstIt dstIt;
};

struct V8ToDBus {
    typedef V8ObjectIterator src;
    typedef DBusIterator     dst;
};

struct DBusToV8 {
    typedef DBusIterator     src;
    typedef V8ObjectIterator dst;
};

/*
    static void convert(Stack & stack) {
        DBusMessageIter * it = & stack.back().it;
        int type = dbus_message_iter_get_arg_type(it);
        std::cerr << "type: " << type << std::endl;
        switch (type) {
            case DBUS_TYPE_INVALID:
                //pop(stack);
                break;
            case DBUS_TYPE_STRING:
                //v = convertString(it());
                break;
            case DBUS_TYPE_ARRAY:
                //v = convertArray(stack, it(), & new_container);
                break;
            case DBUS_TYPE_VARIANT:
                break;
            case DBUS_TYPE_STRUCT:
                break;
            case DBUS_TYPE_DICT_ENTRY:
                break;

            default:
                throw "kaputt";
                break;
        }
    }
*/

template <typename T> struct ctor_arg;

template <>
struct ctor_arg<DBusIterator> {
    static inline
    DBusMessage *
    get(DBusMessage * msg, v8::Handle<v8::Value> obj) {
        return msg;
    }
};

template <>
struct ctor_arg<V8ObjectIterator> {
    static inline
    v8::Handle<v8::Value>
    get(DBusMessage * msg, v8::Handle<v8::Value> obj) {
        return obj;
    }
};

template <typename ConvPolicy>
struct Traverser {

    typedef typename ConvPolicy::src SrcT;
    typedef typename ConvPolicy::dst DstT;
    typedef F<SrcT, DstT> StackFrame;
    typedef std::vector<StackFrame> Stack;

    Traverser(DBusMessage * msg, v8::Handle<v8::Value> obj) {
        stack.push_back(StackFrame(
                      ctor_arg<SrcT>::get(msg, obj)
                    , ctor_arg<DstT>::get(msg, obj)));
    }
    void go() {
        bool needs_pop;
        while ( ! stack.empty() ) {
            needs_pop = stack.back().work(stack);
            if (needs_pop) {
                stack.pop_back();
            }
        }
    }
    Stack stack;
};


} // end of namespace node_dbus
#endif // NODE_DBUS_CONVERTER_INCLUDED
