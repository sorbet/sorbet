#!/bin/bash

set -euo pipefail

what=""
if [ "$#" -eq 0 ]; then
  what="fuzz_dash_e"
elif [ "$#" -eq 1 ]; then
  what="$1"
else
  echo "usage: $0 [<fuzz_target>]"
  exit 1
fi

echo "building $what"
bazel build "//test/fuzz:$what" --config=fuzz -c opt

# normally we'd like to check to see if commands are around before running the script, but in this case, we want the
# bazel build command to run before this check so that bazel can download itself.
echo "checking for commands"
export PATH="$PATH:$PWD/bazel-sorbet/external/llvm_toolchain/bin"
if ! command -v llvm-symbolizer >/dev/null; then
  echo "fatal: command not found: llvm-symbolizer"
  exit 1
fi

echo "setting up files"
mkdir -p fuzz_corpus
find test/testdata -iname "*.rb" | grep -v disable | xargs -n 1 -I % cp % fuzz_corpus
mkdir -p fuzz_crashers/original

echo "running"
export ASAN_OPTIONS="dedup_token_length=10"
nice "./bazel-bin/test/fuzz/$what" \
  -use_value_profile=1 \
  -only_ascii=1 \
  -dict=test/fuzz/ruby.dict \
  -artifact_prefix=fuzz_crashers/original/ \
  fuzz_corpus
