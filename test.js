var t   = require('./lib/tests')
  , sys = require('sys')
  ;

sys.puts("======================================================"
    ,  "total: " + t.total + " failed: " + t.failed);

if ( t.failed > 0) {
  process.exit(1);
}
      

