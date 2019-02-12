#!/bin/bash

set -exuo pipefail

# build the actual fuzz target
bazel build //test/fuzz:fuzz_dash_e --config=fuzz -c opt

export ASAN_OPTIONS=dedup_token_length=10 # use top 10 frames to tell different errors appart


# setup corpus
mkdir -p fuzz_corpus
find ./test/testdata/ -iname "*.rb"|grep -v disable|xargs -n 1 -I % cp % fuzz_corpus

mkdir -p fuzz_crashers/original

# fuzz
nice ./bazel-bin/test/fuzz/fuzz_dash_e -use_value_profile=1 -only_ascii=1 -artifact_prefix=fuzz_crashers/original/ fuzz_corpus/ -jobs=100
