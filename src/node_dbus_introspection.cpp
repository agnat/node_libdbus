#include "node_dbus_config.h"

#include <vector>
#include <node.h>
#include <expat.h>

#include "v8_utils.hpp"

using namespace v8;
using namespace v8_utils;

namespace node_dbus {

namespace expat {

typedef std::vector<Persistent<Object> > Stack;

void
start_element(void *userData, const XML_Char *name, const XML_Char **atts) {
    HandleScope scope;
    Local<Object> element(Object::New());
    element->Set(String::NewSymbol("nodeName"), String::New(name));
    element->Set(String::NewSymbol("children"), Array::New());
    Local<Object> attributes(Object::New());
    element->Set(String::NewSymbol("attributes"), attributes);
    
    const XML_Char * k; const XML_Char * v;
    while (*atts) {
        k = *atts++;
        v = *atts++;
        attributes->Set(String::NewSymbol(k), String::New(v));
    }

    Stack * stack = static_cast<Stack*>(userData);
    Local<Array> children =
        Array::Cast(*stack->back()->Get(String::NewSymbol("children")));
    children->Set(children->Length(), element);
    stack->push_back(Persistent<Object>::New(element));
}

void
end_element(void *userData, const XML_Char *name) {
    Stack * stack = static_cast<Stack*>(userData);
    stack->back().Dispose();
    stack->pop_back();
}

Handle<Value>
xml_to_js(Handle<String> xmlString) {
    HandleScope scope;
    std::vector<Persistent<Object> > stack;
    Local<Object> root(Object::New());
    Local<Object> children(Array::New());

    root->Set(String::NewSymbol("children"), children);
    stack.push_back(Persistent<Object>::New(root));

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, & stack);
    XML_SetElementHandler(parser, start_element, end_element);

    String::Utf8Value utf8(xmlString);
    bool isDone(false);
    XML_Status status = XML_Parse(parser, *utf8, utf8.length(), isDone);
    if (status != XML_STATUS_OK) {
        std::ostringstream msg;
        msg << "failed to parse dbus introspection XML: "
            << XML_ErrorString(XML_GetErrorCode(parser));
        return throwError(msg.str().c_str());
    }

    XML_ParserFree(parser);

    stack.back().Dispose();
    stack.pop_back();

    return scope.Close(children->Get(0));
}

} // end of namespace expat

Handle<Value>
introspectionXmlToJs(Arguments const& args) {
    HandleScope scope;
    if (argumentCountMismatch(args, 1)) {
        return throwArgumentCountMismatchException(args, 1);
    }
    if ( ! args[0]->IsString()) {
        return throwTypeError("argument 1 must be a string (introspection XML)");
    }
    return scope.Close(expat::xml_to_js(args[0]->ToString()));
}

} // end of namespace node_dbus
