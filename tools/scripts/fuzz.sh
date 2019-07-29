#!/bin/bash

set -exuo pipefail

if ! command -v llvm-symbolizer >/dev/null; then
  echo "fatal: command not found: llvm-symbolizer"
  exit 1
fi

bazel build //test/fuzz:fuzz_dash_e --config=fuzz -c opt
export PATH="$PATH:$(pwd)/bazel-sorbet/external/llvm_toolchain/bin"
export ASAN_OPTIONS='dedup_token_length=10'

mkdir -p fuzz_corpus
find test/testdata -iname "*.rb" | grep -v disable | xargs -n 1 -I % cp % fuzz_corpus

mkdir -p fuzz_crashers/original

nice ./bazel-bin/test/fuzz/fuzz_dash_e \
  -use_value_profile=1 \
  -only_ascii=1 \
  -dict=test/fuzz/ruby.dict \
  -artifact_prefix=fuzz_crashers/original/ \
  fuzz_corpus
