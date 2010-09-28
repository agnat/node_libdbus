#!/usr/bin/env node

var dbus = require('./lib/binding'),
    sys  = require('sys');

sys.puts(sys.inspect(dbus));

var c = new dbus.Connection();
