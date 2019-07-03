#!/bin/bash

set -exuo pipefail

# build the actual fuzz target
bazel build //test/fuzz:fuzz_dash_e --config=fuzz -c opt
PATH=$PATH:$(pwd)/bazel-sorbet/external/llvm_toolchain/bin/
export PATH
export ASAN_OPTIONS=dedup_token_length=10 # use top 10 frames to tell different errors appart


# setup corpus
mkdir -p fuzz_corpus
find ./test/testdata/ -iname "*.rb"|grep -v disable|xargs -n 1 -I % cp % fuzz_corpus

mkdir -p fuzz_crashers/original

command -v llvm-symbolizer >/dev/null 2>&1 || { echo 'will need llvm-symbolizer' ; exit 1; }

# fuzz
nice ./bazel-bin/test/fuzz/fuzz_dash_e -use_value_profile=1 -only_ascii=1 -dict=test/fuzz/ruby.dict -artifact_prefix=fuzz_crashers/original/ fuzz_corpus/ --stress-incremental-resolver
