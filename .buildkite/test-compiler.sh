#!/bin/bash

set -euo pipefail

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == "$platform" ]]; then
    apt-get update
    apt-get install -yy libncurses5-dev libncursesw5-dev xxd
elif [[ "mac" == "$platform" ]]; then
    if ! [ -x "$(command -v wget)" ]; then
        brew install wget
    fi
fi

export JOB_NAME=test
source .buildkite/tools/setup-bazel.sh

err=0

# Build sorbet_ruby once with gcc, to ensure that we can build it without depending on the clang toolchain in the
# sandbox
echo "--- building ruby with gcc"
./bazel build @sorbet_ruby_2_7_for_compiler//:ruby --crosstool_top=@bazel_tools//tools/cpp:toolchain

echo "+++ running tests"

mkdir -p _out_

# `-c opt` is required, otherwise the tests are too slow
# forcedebug is really the ~only thing in `--config=dbg` we care about.
# must come after `-c opt` because `-c opt` will define NDEBUG on its own
./bazel test //test:compiler //test/cli/compiler \
  --experimental_generate_json_trace_profile --profile=_out_/profile.json \
  -c opt \
  --config=forcedebug \
  --test_summary=terse \
  --spawn_strategy=local \
  --test_output=errors || err=$?

# --- post process test results here if you want ---

if [ "$err" -ne 0 ]; then
    exit "$err"
fi
