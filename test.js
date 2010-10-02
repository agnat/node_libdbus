#!/usr/bin/env node

var dbus = require('./lib/dbus'),
    sys  = require('sys');

var c = new dbus.Connection(dbus.DBUS_BUS_SYSTEM);

var msg = dbus.createMethodCall(
    dbus.DBUS_SERVICE_DBUS
  , dbus.DBUS_PATH_DBUS
  , dbus.DBUS_INTERFACE_DBUS
  , "ListNames"
);


var call = c.sendWithReply(msg);
call.setNotify(function(message){
  sys.puts("==== plöng")
  sys.puts(sys.inspect(message))
});

setTimeout(function(){
  c.close();
  c = null
}, 2000);

setTimeout(function(){}, 7000);
