#!/bin/bash

set -euo pipefail

what=""
if [ "$#" -eq 0 ]; then
  what="fuzz_dash_e"
elif [ "$#" -eq 1 ]; then
  what="$1"
else
  echo "usage: $0 <fuzz_target>"
  exit 1
fi

echo "setting env vars"
export PATH="$PATH:$(pwd)/bazel-sorbet/external/llvm_toolchain/bin"
export ASAN_OPTIONS='dedup_token_length=10'

echo "checking for commands"
if ! command -v llvm-symbolizer >/dev/null; then
  echo "fatal: command not found: llvm-symbolizer"
  exit 1
fi

echo "building $what"
bazel build "//test/fuzz:$what" --config=fuzz -c opt

echo "setting up files"
mkdir -p fuzz_corpus
find test/testdata -iname "*.rb" | grep -v disable | xargs -n 1 -I % cp % fuzz_corpus
mkdir -p fuzz_crashers/original

echo "running"
nice "./bazel-bin/test/fuzz/$what" \
  -use_value_profile=1 \
  -only_ascii=1 \
  -dict=test/fuzz/ruby.dict \
  -artifact_prefix=fuzz_crashers/original/ \
  fuzz_corpus
