var util  = require('util')
  , dbus = require('./binding')
  ;

var IOWatcher = process.binding('io_watcher').IOWatcher;

var DBusIO = {}

DBusIO._addWatch = function(dbus_watch) {
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
    // XXX the dispatch stuff belongs to connection only ... not to server
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

DBusIO._removeWatch = function(dbus_watch) {
  dbus_watch.io_watcher.stop();
}

DBusIO._toggleWatch = function(dbus_watch) {
  util.puts("toggleWatch() NOT IMPLEMENTED");
}

DBusIO._addTimeout = function(dbus_timeout) {
  dbus_timeout.nodeTimeoutId = setTimeout(function() {
    dbus_timeout.handle();
  }, dbus_timeout.interval);
}

DBusIO._removeTimeout = function(dbus_timeout) {
  clearTimeout(dbus_timeout.nodeTimeoutId);
  dbus_timeout.nodeTimeoutId = null;
}

DBusIO._toggleTimeout = function(dbus_timeout) {
  if (dbus_timeout.nodeTimeoutId) {
    this.removeTimeout(dbus_timeout);
  } else {
    this.addTimeout(dbus_timeout);
  }
}

exports.addWatchAndTimeoutFunctions = function(ctor) {
  for (f in DBusIO) {
    ctor.prototype[f] = DBusIO[f];
  }
}
