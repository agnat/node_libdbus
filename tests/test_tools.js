var assert = require('assert')
  , util = require('util')
  ;

for (var thing in assert) {
  exports[thing] = assert[thing];
}

exports.TestTimer = function(timeout, expected_hits, done_callback) {
  var self = this;
  self.timeoutId = setTimeout(function() {
    assert.ok(false, "Timeout: Only " + self.hits + " of " + expected_hits
      + " hits seen within " + (timeout/1000) + " seconds.");
  }, timeout);
  self.expected_hits = expected_hits;
  self.hits = 0;
  self.messages = [];
  self.callback = done_callback;
  process.on('exit', function() {
    assert.strictEqual(self.expected_hits, self.hits);
  });
}

exports.TestTimer.prototype.passed = function(msg) {
  this.hits += 1;
  this.messages.push(msg);
  if (this.hits === this.expected_hits) {
    clearTimeout(this.timeoutId);
    this.callback();
  }
}


