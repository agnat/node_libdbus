#!/usr/bin/env node

var   dbus   = require('../lib/dbus')
    , sys    = require('sys')
    , assert = require('assert')
    ;

var msg = dbus.createMethodCall(
    dbus.DBUS_SERVICE_DBUS
  , dbus.DBUS_PATH_DBUS
  , dbus.DBUS_INTERFACE_DBUS
  , "SomeMethod"
);

msg.appendArgs('foo');
