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
  var handle_reads = flags & dbus.DBUS_WATCH_READABLE ? true : false;
  var handle_writes = flags & dbus.DBUS_WATCH_WRITABLE ? true : false;
  watcher.set(dbus_watch.unixFd(), handle_reads, handle_writes);
  watcher.callback = function() {
    var dbus_flags = (handle_reads ? dbus.DBUS_WATCH_READABLE : 0 )
        | (handle_writes ? dbus.DBUS_WATCH_WRITABLE : 0);
    dbus_watch.handle(dbus_flags);
    var dispatcher = function() {
      watcher.host.dispatch();
      if (watcher.host.dispatchStatus == dbus.DBUS_DISPATCH_DATA_REMAINS) {
        process.nextTick(dispatcher);
      }
    }
    dispatcher();
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

dbus.Connection.prototype.addTimeout = function(dbus_timeout) {
  dbus_timeout.nodeTimeoutId = setTimeout(function() {
    dbus_timeout.handle();
  }, dbus_timeout.interval);
}

dbus.Connection.prototype.removeTimeout = function(dbus_timeout) {
  clearTimeout(dbus_timeout.nodeTimeoutId);
  dbus_timeout.nodeTimeoutId = null;
}

dbus.Connection.prototype.toggleTimeout = function(dbus_timeout) {
  if (dbus_timeout.nodeTimeoutId) {
    this.removeTimeout(dbus_timeout);
  } else {
    this.addTimeout(dbus_timeout);
  }
}

dbus.Connection.prototype.toggleTimeout = function(dbus_timeout) {
  sys.puts("===== Dispatch status changed =====");
}
//=============================================================================

dbus.Message.prototype.isError = function() {
  return this.type == dbus.DBUS_MESSAGE_TYPE_ERROR;
}

//=============================================================================

dbus.systemBus = function() {
  return new dbus.Connection(dbus.DBUS_BUS_SYSTEM);
}
dbus.sessionBus = function() {
  return new dbus.Connection(dbus.DBUS_BUS_SESSION);
}

//=============================================================================

exports.Connection         = dbus.Connection;
exports.Message            = dbus.Message; 
exports.createMethodCall   = dbus.createMethodCall;
exports.createMethodReturn = dbus.createMethodReturn;
exports.createErrorMessage = dbus.createErrorMessage;
exports.createSignal       = dbus.createSignal;

