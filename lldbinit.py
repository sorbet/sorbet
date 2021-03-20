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
bazel_workspace = os.path.realpath(os.path.join(project_root, "bazel-sorbet"))

if not(os.path.isdir(bazel_workspace)):
    print("You need to compile Sorbet first before loading this file")
else:
    ci = lldb.debugger.GetCommandInterpreter()
    print("(lldb) Mapping paths in ./bazel-sorbet to the project root")

    source_map_command = 'settings set -- target.source-map "%s" "%s"' % (bazel_workspace, project_root)
    res = lldb.SBCommandReturnObject()
    ci.HandleCommand(source_map_command, res)

    source_map_command = 'settings set -- target.source-map "/proc/self/cwd" "%s"' % bazel_workspace
    res = lldb.SBCommandReturnObject()
    ci.HandleCommand(source_map_command, res)
