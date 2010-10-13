#!/usr/bin/env node

var   dbus = require('../lib/dbus')
    , util  = require('util')
    , tt   = require('./test_tools')
    ;

var timer = new tt.TestTimer(100, 2, function() { bus.close() });

var bus = dbus.sessionBus();

bus.requestName('org.example.NodeDBusTest');
bus.registerObjectPath('/', function(msg) {
  timer.passed('remote call received');
  //util.puts(util.inspect(msg));
  //util.puts(util.inspect(msg.args()));

  tt.ok( ! msg.isError());
  tt.strictEqual(msg.signature, 'is(is)');

  var args = msg.args();
  tt.strictEqual(args.length, 3);
  tt.strictEqual(args[0], 5);
  tt.strictEqual(args[1], 'foo');
  tt.ok(Array.isArray(args[2]));
  tt.strictEqual(args[2][0], 23);
  tt.strictEqual(args[2][1], 'bar');

  var result = dbus.createMethodReturn(msg);
  result.appendArgs("foo");
  bus.send(result);
});

var msg = dbus.createMethodCall(
    'org.example.NodeDBusTest'
  , '/'
  , 'org.example.NodeDBusTest'
  , "foo"
);

msg.appendArgs(5, "foo", [23, "bar"]);
bus.send(msg, function(result) {
  //util.puts(util.inspect(message));
  //util.puts(util.inspect(message.args()));
  timer.passed('remote call result received');
  tt.strictEqual(result.signature, 's');
  var args = result.args();
  tt.strictEqual(args.length, 1);
  tt.strictEqual(args[0], 'foo');
});

