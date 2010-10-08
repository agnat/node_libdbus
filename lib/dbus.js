var dbus = require('./binding'),
    sys = require('sys');

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

function handleMethod(method_data) {
  var m = {
      name: method_data.attributes['name']
    , signature: {
        args: [],
        result: []
      }
  };
  method_data.children.forEach(function(child) {
    switch (child.nodeName) {
      case 'arg':
        var a = handleArg(child);
        if (a.direction === 'in') {
          m.signature.args.push(a);
        } else {
          m.signature.result.push(a);
        }
        break;
      default:
        sys.puts("UNHANDLED: " + child.nodeName);
        break;
    }
  });
  return m;
}

function handleSignal(signal_data) {
  var s = { name: signal_data.attributes['name'] };
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
        sys.puts("UNHANDLED: " + child.nodeName);
        break;
    }
  });
  return iface;
}

dbus.Proxy = function(destination, path, introspectionData, connection) {
  var self = this;
  if (introspectionData.nodeName !== 'node') {
    sys.puts("kaputt: expeceted 'node' but got '" + introspectionData.nodeName + "'");
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
        sys.puts("UNHANDLED: " + child.nodeName);
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
      sys.puts("=== call " + method.name
          + " argc: " + method.signature.args.length
          + " resultc: " + method.signature.result.length
      );
      var cb = arguments[arguments.length - 1];
      if (typeof cb !== 'function') {
        throw Error('result callback missing in method invocation');
      }
      var call = dbus.createMethodCall(obj.destination, obj.name, iface, method.name);
      obj.connection.send(call, cb);
    }
}

dbus.getInterface = function(obj, interface_name) {
  if ( !( interface_name in obj.interfaces)) {
    throw new Error("object " + obj.name + " does not implement interafce "
        + interface_name); 
  }

  var iface = obj.interfaces[interface_name];
  var o = {};
  for (var m in iface.methods) {
    var methodName = nodeMethodName(m)
    o[methodName] = callAdapter(obj, interface_name, iface.methods[m]);
  }
  return o;
}

//=== Connection =========================================
dbus.Connection.prototype.getObject = function(destination, path, callback) {
  var self = this;
  self.introspect(destination, path, function(introspection_data) {
    callback( new dbus.Proxy(destination, path, introspection_data, self));
  });
}

dbus.Connection.prototype.introspect = function(destination, path, callback) {
  var msg = dbus.createMethodCall(
        destination
      , path
      , dbus.DBUS_INTERFACE_INTROSPECTABLE
      , 'Introspect');
  this.send(msg, function(result) {
    var xmlString = result.args().shift();
    var pseudoDOM = dbus.introspectionXmlToJs(xmlString);
    callback( pseudoDOM);
  });
}

//=== private node <-> dbus main loop glue ===============

var IOWatcher = process.binding('io_watcher').IOWatcher;

dbus.Connection.prototype._addWatch = function(dbus_watch) {
  var watcher = new IOWatcher();
  watcher.host = this;
  var flags = dbus_watch.flags();
  var handle_reads = flags & dbus.DBUS_WATCH_READABLE ? true : false;
  var handle_writes = flags & dbus.DBUS_WATCH_WRITABLE ? true : false;
  watcher.set(dbus_watch.unixFd(), handle_reads, handle_writes);
  watcher.callback = function() {
    var dbus_flags = (handle_reads ? dbus.DBUS_WATCH_READABLE : 0 )
        | (handle_writes ? dbus.DBUS_WATCH_WRITABLE : 0);
    dbus_watch.handle(dbus_flags);
    var dispatcher = function() {
      watcher.host.dispatch();
      if (watcher.host.dispatchStatus == dbus.DBUS_DISPATCH_DATA_REMAINS) {
        process.nextTick(dispatcher);
      }
    }
    dispatcher();
  }
  if (dbus_watch.enabled()) {
    watcher.start();
  }
  dbus_watch.io_watcher = watcher;
}

dbus.Connection.prototype._removeWatch = function(dbus_watch) {
  dbus_watch.io_watcher.stop();
}

dbus.Connection.prototype._toggleWatch = function(dbus_watch) {
  sys.puts("toggleWatch() NOT IMPLEMENTED");
}

dbus.Connection.prototype._addTimeout = function(dbus_timeout) {
  dbus_timeout.nodeTimeoutId = setTimeout(function() {
    dbus_timeout.handle();
  }, dbus_timeout.interval);
}

dbus.Connection.prototype._removeTimeout = function(dbus_timeout) {
  clearTimeout(dbus_timeout.nodeTimeoutId);
  dbus_timeout.nodeTimeoutId = null;
}

dbus.Connection.prototype._toggleTimeout = function(dbus_timeout) {
  if (dbus_timeout.nodeTimeoutId) {
    this.removeTimeout(dbus_timeout);
  } else {
    this.addTimeout(dbus_timeout);
  }
}

dbus.Connection.prototype._dispatchStatusChanged = function(dbus_timeout) {
  sys.puts("===== Dispatch status changed =====");
}
//=============================================================================

dbus.Message.prototype.isError = function() {
  return this.type == dbus.DBUS_MESSAGE_TYPE_ERROR;
}

//=============================================================================

dbus.systemBus = function() {
  return new dbus.Connection(dbus.DBUS_BUS_SYSTEM);
}
dbus.sessionBus = function() {
  return new dbus.Connection(dbus.DBUS_BUS_SESSION);
}

//=============================================================================

exports.Connection         = dbus.Connection;
exports.Message            = dbus.Message; 
exports.StandardInterfaces = dbus.StandardInterfaces;

exports.createMethodCall   = dbus.createMethodCall;
exports.createMethodReturn = dbus.createMethodReturn;
exports.createErrorMessage = dbus.createErrorMessage;
exports.createSignal       = dbus.createSignal;

exports.systemBus = dbus.systemBus;
exports.sessionBus = dbus.sessionBus;

exports.getInterface = dbus.getInterface;
