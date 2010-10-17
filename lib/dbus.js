var dbus  = require('./binding')
  , util   = require('util')
  , events = require('events')
  ;

for (var c in dbus.constants) {
  exports[c] = dbus.constants[c];
  dbus[c] = dbus.constants[c];
}

//=== Proxy ==============================================

function handleArg(arg_data) {
  return {
      name: arg_data.attributes['name']
    , type: arg_data.attributes['type']
    , direction: arg_data.attributes['direction']
  };
}

function argumentCollector(receiver) {
  return function(child) {
    switch (child.nodeName) {
      case 'arg':
        var a = handleArg(child);
        if (a.direction === 'in') {
          receiver.signature.args.push(a);
        } else {
          receiver.signature.result.push(a);
        }
        break;
      default:
        util.puts("UNHANDLED: " + child.nodeName);
        break;
    }
  }
}

function handleMethod(method_data) {
  var m = {
      name: method_data.attributes['name']
    , signature: {
        args: [],
        result: []
      }
  };
  method_data.children.forEach(argumentCollector(m));
  return m;
}

function handleSignal(signal_data) {
  var s = { name: signal_data.attributes['name'] , signature: { result: [] }};
  signal_data.children.forEach(argumentCollector(s));
  return s;
}

function handleInterface(ifdata) {
  var iface = {
      name: ifdata.attributes['name']
    , methods: {}
    , signals: {}
  };
  ifdata.children.forEach(function(child) {
    switch (child.nodeName) {
      case 'method':
        var m = handleMethod(child);
        iface.methods[m.name] = m;
        break;
      case 'signal':
        var s = handleSignal(child);
        iface.signals[s.name] = s;
        break;
      default:
        util.puts("UNHANDLED: " + child.nodeName);
        break;
    }
  });
  return iface;
}

dbus.Proxy = function(destination, path, introspectionData, connection) {
  var self = this;
  if (introspectionData.nodeName !== 'node') {
    util.puts("kaputt: expeceted 'node' but got '" + introspectionData.nodeName + "'");
  }
  self.name = introspectionData.attributes['name'] || path;
  self.interfaces = {};
  self.children = [];
  self.connection = connection;
  self.destination = destination;
  
  introspectionData.children.forEach(function(child) {
    switch (child.nodeName) {
      case 'interface':
        var iface = handleInterface(child);
        self.interfaces[iface.name] = iface;
        break;
      default:
        util.puts("UNHANDLED: " + child.nodeName);
        break;
    }
  });
}

function nodeMethodName(dbus_method) {
  var first_char = dbus_method.substr(0, 1).toLowerCase();
  return first_char + dbus_method.substr(1);
}

function callAdapter(obj, iface, method) {
    return function() {
      var args = Array.prototype.slice.call(arguments);
      var cb = args.pop();
      if (typeof cb !== 'function') {
        throw Error('result callback missing in method invocation');
      }
      var call = dbus.createMethodCall(obj.destination, obj.name, iface, method.name);
      call.appendArgsWithSignature(args, method.signature.args.join(''));
      obj.connection.send(call, function(result) {
        if (result.isError()) {
          // TODO extract error
          cb.call(this, result);
        } else {
          var args = result.args();
          args.unshift(null);
          cb.apply(this, args);
        }
      });
    }
}

dbus.getInterface = function(obj, interface_name) {
  if ( !( interface_name in obj.interfaces)) {
    throw new Error("object " + obj.name + " does not implement interafce "
        + interface_name); 
  }

  var iface = obj.interfaces[interface_name];
  var o = new events.EventEmitter();
  for (var m in iface.methods) {
    var name = nodeMethodName(m)
    o[name] = callAdapter(obj, interface_name, iface.methods[m]);
  }
  return o;
}


dbus.Message.prototype.isError = function() {
  return this.type == dbus.DBUS_MESSAGE_TYPE_ERROR;
}

//=== Server ==================================================================

dbus.Server.prototype._onNewConnection = function(connection) {
  util.puts('_onNewConnection');
}

//=============================================================================

var Connection = require('./connection').Connection;

exports.systemBus = function() {
  return new Connection(dbus.DBUS_BUS_SYSTEM);
}
exports.sessionBus = function() {
  return new Connection(dbus.DBUS_BUS_SESSION);
}

//=============================================================================

exports.Connection         = Connection;
exports.Message            = dbus.Message; 
//exports.Server             = dbus.Server;

exports.createMethodCall   = dbus.createMethodCall;
exports.createMethodReturn = dbus.createMethodReturn;
exports.createErrorMessage = dbus.createErrorMessage;
exports.createSignal       = dbus.createSignal;

exports.getInterface = dbus.getInterface;
