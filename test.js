#!/usr/bin/env node

var dbus = require('./lib/dbus'),
    sys  = require('sys');

sys.puts(sys.inspect(dbus));

var c = new dbus.Connection(dbus.DBUS_BUS_SYSTEM);
sys.puts(sys.inspect(c));

setTimeout(function(){
  c.close();
  c = null
}, 5000);
setTimeout(function(){}, 10000);
