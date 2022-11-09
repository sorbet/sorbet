#!/bin/bash
script="$1"
expect="$2"

export ASAN_SYMBOLIZER_PATH=`pwd`/external/llvm_toolchain_12_0_0/bin/llvm-symbolizer

if ! diff "$expect" -u <("$script"); then
  cat <<EOF
================================================================================
There were differences in the captured output when running this CLI test.
To make this output the expected output, run this and commit the changes:
./bazel test //test/cli:update
EOF
  exit 1
fi
