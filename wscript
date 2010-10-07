import sys
import Options
import os

# nice, but requires python 2.6 ... 
#import json
#package = json.load(open('package.json'))
#APPNAME = 'node_' + package['name'] # used by 'node-waf dist'
#VERSION = package['version']        # dito

APPNAME = 'node_dbus'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')
  opt.tool_options('compiler_cc')
  opt.tool_options('node_addon')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('compiler_cc')
  conf.check_tool('node_addon')

  conf.check_cfg(
      package='dbus-1'
    , args='--cflags --libs'
    , uselib_store='DBUS'
  )

  conf.check_cc(lib='expat', uselib_store='EXPAT', mandatory=True)
  conf.check_cc(header_name='expat.h', mandatory=True)

  conf.write_config_header('node_dbus_config.h');


def post_build(ctx):
  #print("=== post")
  if not os.path.exists('lib/binding.node'):
      os.symlink( '../build/default/binding.node', 'lib/binding.node')
  if not os.path.exists('lib/tests.node'):
      os.symlink( '../build/default/tests.node', 'lib/tests.node')

def build(bld):
  bld.add_post_fun(post_build)
  addon = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  addon.target = 'binding'
  addon.uselib = ['DBUS', 'EXPAT']
  addon.include = '.'
  addon.source = [
      'src/binding.cpp'
    , 'src/node_dbus_connection.cpp'
    , 'src/node_dbus_utils.cpp'
    , 'src/node_dbus_watch.cpp'
    , 'src/node_dbus_timeout.cpp'
    , 'src/node_dbus_message.cpp'
    , 'src/node_dbus_pending_call.cpp'
    , 'src/node_dbus_introspection.cpp'
  ]

  tests = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  tests.target = 'tests'
  tests.uselib = 'DBUS'
  tests.source = 'src/tests.cpp'
  
# vim: set filetype=python :
