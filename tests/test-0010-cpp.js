#!/usr/bin/env node

var t   = require('../lib/tests')
  , util = require('util')
  ;

if ( t.failed > 0) {
  process.exit(1);
}
      

