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
    import lldb
except:
    pass

import re
import os

# Bazel builds all binaries from a username-specific cache location
# for lldb to work properly we need to map that path back to our main source
project_root = os.path.dirname(os.path.realpath(__file__))
# Bazel names its convenience symlinks after the directory containing the
# workspace, which isn't `sorbet` in a worktree or a renamed checkout.
bazel_symlink = "bazel-%s" % os.path.basename(project_root)
if not(os.path.isdir(os.path.join(project_root, bazel_symlink))):
    bazel_symlink = "bazel-sorbet"
bazel_workspace = os.path.realpath(os.path.join(project_root, bazel_symlink))

# `bazel_workspace` is `<output_base>/execroot/<workspace name>`. Its `external`
# directory only links the repos that the most recent build needed, so map to
# `<output_base>/external`, which has all of them.
external_path = os.path.join(os.path.dirname(os.path.dirname(bazel_workspace)), "external")

# It seems like macOS builds get absolute paths in the debug info.
#
# Linux builds with PWD=/proc/self/cwd, which lldb then resolves against its
# own PWD, not the build sandbox, so we need to map them to the real path
# (inside the bazel workspace).
#
# It's safe for macOS to see an extra mapping here, it will just never be used.
#
# NOTE: settings set replaces the full array setting, so we have to pass
# everything we want in a single command. We could consider changing this to
# e.g. `append` if we wanted something more robust, but there are edge cases
# there, too.
source_map_command = 'settings set -- target.source-map "%s" "%s" /proc/self/cwd/external "%s"' % (
    bazel_workspace, project_root, external_path
)

if not(os.path.isdir(bazel_workspace)):
    print("You need to compile Sorbet first before loading this file")
else:
    print("(lldb) %s" % source_map_command)
    ci = lldb.debugger.GetCommandInterpreter()
    res = lldb.SBCommandReturnObject()
    ci.HandleCommand(source_map_command, res)


def __lldb_init_module(debugger, internal_dict):
    result = lldb.SBCommandReturnObject()
    ci = debugger.GetCommandInterpreter()
    ci.HandleCommand("command script add -f lldbinit.cmd_bazelsourcemap bazelsourcemap", result)


def cmd_bazelsourcemap(debugger, command, result, dict):
    # Loading this file already applies the source map. This command applies it
    # again, e.g. after something else has changed target.source-map.
    ci = debugger.GetCommandInterpreter()
    print('(lldb) %s' % source_map_command)
    ci.HandleCommand(source_map_command, result)

