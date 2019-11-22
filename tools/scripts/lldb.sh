#!/bin/bash
set -e

bazel build @ruby_2_6_3//:ruby --config dbg

llvmir=$1
rb=$2
shift
shift
if [ "${rb:0:1}" != "/" ]; then
    rb="./$rb"
fi

RUBYLIB=./bazel-bin/external/ruby_2_6_3/ruby.runfiles/ruby_2_6_3/lib/ruby/2.6.0/x86_64-darwin18:./bazel-bin/external/ruby_2_6_3/ruby.runfiles/ruby_2_6_3/lib/ruby/2.6.0 llvmir=$llvmir lldb -- ./bazel-bin/external/ruby_2_6_3/ruby.runfiles/ruby_2_6_3/bin/ruby -W0 -r ./run/tools/preamble.rb -r ./run/tools/patch_require.rb -e "require '$rb'" "$@"
