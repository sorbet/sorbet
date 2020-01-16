#!/bin/bash
set -e

bazel build @sorbet_ruby//:ruby --config dbg

llvmir=$1
rb=$2
shift
shift
if [ "${rb:0:1}" != "/" ]; then
    rb="./$rb"
fi

llvmir=$llvmir lldb -- ./bazel-bin/external/sorbet_ruby/toolchain/bin/ruby \
  -r ./run/tools/preamble.rb \
  -r ./run/tools/patch_require.rb \
  -e "require '$rb'" \
  "$@"
