var dbus = require('./binding')
  , dbus_io = require('./dbus_io')
  ;

dbus_io.addWatchAndTimeoutFunctions(dbus.Connection);

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

dbus.Connection.prototype._dispatchStatusChanged = function(new_status) {
  /*
  util.puts("===== Dispatch status changed =====");
  switch (new_status) {
    case dbus.DBUS_DISPATCH_DATA_REMAINS: util.puts("dispatch data remains"); break;
    case dbus.DBUS_DISPATCH_COMPLETE: util.puts("dispatch complete"); break;
    case dbus.DBUS_DISPATCH_NEED_MEMORY: util.puts("dispatch need memory"); break;
  }
  */
}

exports.Connection = dbus.Connection;
