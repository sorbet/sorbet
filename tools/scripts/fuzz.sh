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

# we want the bazel build command to run before this check so that bazel can download itself.
export PATH="$PATH:$PWD/bazel-sorbet/external/llvm_toolchain/bin"
if ! command -v llvm-symbolizer >/dev/null; then
  echo "fatal: command not found: llvm-symbolizer"
  exit 1
fi

echo "setting up files"
mkdir -p fuzz_corpus
find test/testdata -iname "*.rb" | grep -v disable | xargs -n 1 -I % cp % fuzz_corpus
mkdir -p fuzz_crashers/original

# use top 10 frames to tell different errors apart
export ASAN_OPTIONS="dedup_token_length=10"
# set FUZZ_ARGS to a space-delimited list of other args to pass to the fuzzer, for instance
# --stress-incremental-resolver. by default it is empty.
FUZZ_ARGS="${FUZZ_ARGS:-}"

echo "running with FUZZ_ARGS: $FUZZ_ARGS"
nice "./bazel-bin/test/fuzz/$what" \
  -use_value_profile=1 \
  -only_ascii=1 \
  -dict=test/fuzz/ruby.dict \
  -artifact_prefix=fuzz_crashers/original/ \
  fuzz_corpus \
  $FUZZ_ARGS
