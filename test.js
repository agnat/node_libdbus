#!/usr/bin/env node

var dbus = require('./lib/dbus'),
    sys  = require('sys');

sys.puts(sys.inspect(dbus));

var c = new dbus.Connection(dbus.DBUS_BUS_SYSTEM);
sys.puts(sys.inspect(c));

var msg = dbus.createMethodCall(
    dbus.DBUS_SERVICE_DBUS
  , dbus.DBUS_PATH_DBUS
  , dbus.DBUS_INTERFACE_DBUS
  , "ListNames"
);

sys.puts(sys.inspect(msg));

var call = c.sendWithReply(msg);
call.setNotify(function(){sys.puts("plöng")});

sys.puts("pending call: " + sys.inspect(call));

setTimeout(function(){
  sys.puts("pending call: " + sys.inspect(call));
  c.close();
  sys.puts(sys.inspect(c));
  c = null
}, 5000);
setTimeout(function(){}, 10000);
