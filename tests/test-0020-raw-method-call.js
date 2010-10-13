#!/usr/bin/env node

var   dbus   = require('../lib/dbus')
    , util    = require('util')
    , assert = require('assert')
    ;

var bus = dbus.sessionBus();

var msg = dbus.createMethodCall(
    dbus.DBUS_SERVICE_DBUS
  , dbus.DBUS_PATH_DBUS
  , dbus.DBUS_INTERFACE_DBUS
  , "ListNames"
);

var timeout = setTimeout(function(){
  assert.ok(false, "timeout");
  process.exit(1);
}, 2000);

var gotReply = false;


var pending_call = bus.send(msg, function(message){
  gotReply = true;
  assert.strictEqual(message.type, dbus.DBUS_MESSAGE_TYPE_METHOD_RETURN);
  var result = message.args();
  assert.strictEqual(typeof result, 'object');
  assert.strictEqual(result.length, 1);
  assert.ok(result[0].length > 0);
  var serviceNames = result[0];
  //util.puts(util.inspect(serviceNames));
  var foundDBus = false;
  for (var i = 0; i < serviceNames.length; ++i) {
    if (serviceNames[i] === 'org.freedesktop.DBus') {
      foundDBus = true;
      break;
    }
  }
  assert.ok(foundDBus);
  clearTimeout(timeout);
  bus.close();
  process.exit(0);
});

