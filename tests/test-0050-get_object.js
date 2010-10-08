#!/usr/bin/env node

var dbus   = require('../lib/dbus.js')
  , sys    = require('sys')
  , assert = require('assert')
  ;

var bus = dbus.systemBus();
var timeout = setTimeout(function() {
  assert.ok(false, "timeout");
  process.exit(1);
}, 5000);

var expectedResults = 3;
var resultCount = 0;
function onTestDone() {
  resultCount += 1;
  if (expectedResults === resultCount) {
    bus.close();
    clearTimeout(timeout);
    process.exit(0);
  }
}

bus.introspect('org.freedesktop.DBus', dbus.DBUS_PATH_DBUS, function(data) {
  assert.strictEqual(data.nodeName, 'node');
  onTestDone();
});

bus.getObject('org.freedesktop.DBus', dbus.DBUS_PATH_DBUS, function(obj) {
  assert.strictEqual(obj.name, dbus.DBUS_PATH_DBUS);
  var o = dbus.getInterface(obj, dbus.DBUS_INTERFACE_DBUS);
  o.listNames(function(result) {
    sys.puts(sys.inspect(result.args()));
    onTestDone();
  });
  onTestDone();
});

/*
 
var obj = bus.getObject(
    'org.freedesktop.DBus'
  , dbus.DBUS_PATH_DBUS
  , function(result) { 
    sys.puts(sys.inspect(result));
    sys.puts(result.args());
    bus.close();
  }
);

var obj = bus.getObject(
    'org.freedesktop.Avahi'
  , '/'
  , function(result) { 
    sys.puts(sys.inspect(result));
    sys.puts(result.args());
    bus.close();
  }
);

*/
