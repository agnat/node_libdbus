#ifndef NODE_DBUS_CONVERTER_INCLUDED
#define NODE_DBUS_CONVERTER_INCLUDED

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


} // end of namespace node_dbus
#endif // NODE_DBUS_CONVERTER_INCLUDED
