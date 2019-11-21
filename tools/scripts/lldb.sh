#!/bin/bash
set -e

bazel build @ruby_2_6_3//:ruby --config dbg

RUBYLIB=./bazel-bin/external/ruby_2_6_3/ruby.runfiles/ruby_2_6_3/lib/ruby/2.6.0/x86_64-darwin18:./bazel-bin/external/ruby_2_6_3/ruby.runfiles/ruby_2_6_3/lib/ruby/2.6.0 llvmir=$1 lldb -- ./bazel-bin/external/ruby_2_6_3/ruby.runfiles/ruby_2_6_3/bin/ruby -W0 -r ./run/tools/preamble.rb -r ./run/tools/patch_require.rb -e "require './$2'"
