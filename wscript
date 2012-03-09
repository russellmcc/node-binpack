import Options
from os import unlink, symlink
from os.path import exists 

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  conf.env.set_variant("Release") 
  conf.env.append_value('CXXFLAGS', ["-g", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"])

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "binpack"
  obj.source = "src/binpack.cpp"
  
def shutdown():
    if Options.commands['clean']:
        if exists('binpack.node'): unlink('binpack.node')
    else:
        if exists('build/Release/binpack.node') and not exists('binpack.node'):
            symlink('build/Release/binpack.node', 'binpack.node')