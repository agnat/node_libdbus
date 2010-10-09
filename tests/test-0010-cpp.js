#!/usr/bin/env node

var t   = require('../lib/tests')
  , sys = require('sys')
  ;

if ( t.failed > 0) {
  process.exit(1);
}
      

