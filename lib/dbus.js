var dbus = require('./binding'),
    sys = require('sys');

for (var c in dbus.constants) {
  exports[c] = dbus.constants[c];
  dbus[c] = dbus.constants[c];
}
var IOWatcher = process.binding('io_watcher').IOWatcher;

dbus.Connection.prototype.addWatch = function(dbus_watch) {
  var watcher = new IOWatcher();
  watcher.host = this;
  var flags = dbus_watch.flags();
  watcher.set(dbus_watch.unixFd(),
      flags & dbus.DBUS_WATCH_READABLE ? true : false,
      flags & dbus.DBUS_WATCH_WRITABLE ? true : false);
  watcher.callback = function(readable, writable) {
    sys.puts("==== watch: " + readable + " " + writable);
    var dbus_flags = (readable ? dbus.DBUS_WATCH_READABLE : 0 )
        | (writable ? dbus.DBUS_WATCH_WRITABLE : 0);
  }
  if (dbus_watch.enabled()) {
    watcher.start();
  }
  dbus_watch.io_watcher = watcher;
}

dbus.Connection.prototype.removeWatch = function(dbus_watch) {
  dbus_watch.io_watcher.stop();
}

dbus.Connection.prototype.toggleWatch = function(dbus_watch) {
  sys.puts("toggleWatch() NOT IMPLEMENTED");
}

exports.Connection = dbus.Connection;

