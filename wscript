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
  opt.tool_options('node_addon')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

  conf.write_config_header('node_dbus_config.h');


def post_build(ctx):
  #print("=== post")
  if not os.path.exists('lib/binding.node'):
      os.symlink( '../build/default/binding.node', 'lib/binding.node')

def build(bld):
  bld.add_post_fun(post_build)
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'binding'
  obj.includes = '.'
  obj.source = ['src/binding.cpp', 'src/connection.cpp']
  
# vim: set filetype=python :
