#!/usr/bin/env node

var dbus = require('../lib/dbus.js')
  , util = require('util')
  , tt   = require('./test_tools')
  ;

var bus = dbus.systemBus();
var test_timer = new tt.TestTimer(10, 3, function() { bus.close() });

bus.introspect('org.freedesktop.DBus', dbus.DBUS_PATH_DBUS, function(data) {
  tt.strictEqual(data.nodeName, 'node');
  test_timer.passed('get introspection data');
});

bus.getObject('org.freedesktop.DBus', dbus.DBUS_PATH_DBUS, function(obj) {
  tt.strictEqual(obj.name, dbus.DBUS_PATH_DBUS);
  var o = dbus.getInterface(obj, dbus.DBUS_INTERFACE_DBUS);
  o.listNames(function(error, names) {
    tt.strictEqual(error, null);
    tt.ok('length' in names);
    tt.ok(names.length > 0);
    //util.puts(util.inspect(names));
    test_timer.passed('result of call to ListNames()');
  });
  test_timer.passed('got object');
});

/*
 
var obj = bus.getObject(
    'org.freedesktop.DBus'
  , dbus.DBUS_PATH_DBUS
  , function(result) { 
    util.puts(util.inspect(result));
    util.puts(result.args());
    bus.close();
  }
);

var obj = bus.getObject(
    'org.freedesktop.Avahi'
  , '/'
  , function(result) { 
    util.puts(util.inspect(result));
    util.puts(result.args());
    bus.close();
  }
);

*/
