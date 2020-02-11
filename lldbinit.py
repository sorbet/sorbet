# TODO(jez) Figure out what to do about this warning:
#
#    error: There is a .lldbinit file in the current directory which is not being read.
#    To silence this warning without sourcing in the local .lldbinit,
#    add the following to the lldbinit file in your home directory:
#        settings set target.load-cwd-lldbinit false
#    To allow lldb to source .lldbinit files in the current working directory,
#    set the value of this variable to true.  Only do so if you understand and
#    accept the security risk.
#
# Options for now:
#
#    lldb --local-lldbinit <normal lldb args...>
#    command script import lldbinit.py
#    co sc i lldbinit.py

if __name__ == "__main__":
    print("Run only as script from LLDB... Not as standalone program!")

try:
    import  lldb
except:
    pass

import re
import os

project_root = os.path.dirname(os.path.realpath(__file__))


def __lldb_init_module(debugger, internal_dict):
    result = lldb.SBCommandReturnObject()
    ci = debugger.GetCommandInterpreter()
    ci.HandleCommand("command script add -f lldbinit.cmd_rubysourcemap rubysourcemap", result)

    cyan="\x1b[0;36m"
    cnone="\x1b[0m"

    print("(lldb) Run %srubysourcemap%s to set up Ruby source maps" % (cyan, cnone))


def cmd_rubysourcemap(debugger, command, result, dict):
    ci = debugger.GetCommandInterpreter()
    ci.HandleCommand('list main', result)
    output = result.GetOutput()

    dir_search = re.search('^File: (.*)/main.c', output)
    if dir_search:
        dirname = dir_search.group(1)
        ruby_path = ('%s/bazel-sorbet_llvm/external/sorbet_ruby' % project_root)
        ci.HandleCommand('settings set -- target.source-map %s %s' % (dirname, ruby_path), result)
