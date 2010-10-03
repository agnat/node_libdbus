#!/usr/bin/env node

var   dbus   = require('../lib/dbus')
    , sys    = require('sys')
    , assert = require('assert')
    ;

var bus = new dbus.Connection(dbus.DBUS_BUS_SYSTEM);

var msg = dbus.createMethodCall(
    dbus.DBUS_SERVICE_DBUS
  , dbus.DBUS_PATH_DBUS
  , dbus.DBUS_INTERFACE_DBUS
  , "ListNames"
);

var timeout = setTimeout(function(){
  assert.ok(false);
  process.exit(1);
}, 2000);

var gotReply = false;

process.on('exit', function() {
  assert.strictEqual(gotReply, true);
  process.exit(0);
});

var pending_call = bus.sendWithReply(msg);
pending_call.setNotify(function(message){
  gotReply = true;
  assert.strictEqual(message.type, dbus.DBUS_MESSAGE_TYPE_METHOD_RETURN);
  var result = message.args();
  assert.strictEqual(typeof result, 'object');
  assert.strictEqual(result.length, 1);
  assert.ok(result[0].length > 0);
  clearTimeout(timeout);
  bus.close();
});

