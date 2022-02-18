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
test_args=(
  "//test:compiler"
  "//test/cli/compiler"
  "-c"
  "opt"
  "--config=forcedebug"
  "--spawn_strategy=local"
)

./bazel test \
  --experimental_generate_json_trace_profile \
  --profile=_out_/profile.json \
  --test_summary=terse \
  --test_output=errors \
  "${test_args[@]}" || err=$?

if [ "$err" -ne 0 ]; then
  echo "--- annotating build result"
  failing_tests="$(mktemp)"

  echo 'Run this command to run failing tests locally:' >> "$failing_tests"
  echo >> "$failing_tests"
  echo '```bash' >> "$failing_tests"
  echo "./bazel test \\" >> "$failing_tests"

  # Take the lines that start with target labels.
  # Lines look like "//foo  FAILED in 10s"
  { ./bazel test --test_summary=terse "${test_args[@]}" || true ; } | \
    grep '^//' | \
    sed -e 's/ .*/ \\/' | \
    sed -e 's/^/  /' >> "$failing_tests"

  # Put this last as an easy way to not have a `\` on the last line.
  echo '  -c opt --config=forcedebug' >> "$failing_tests"
  echo '```' >> "$failing_tests"

  buildkite-agent annotate --context "test-static-sanitized.sh" --style error --append < "$failing_tests"

  exit "$err"
fi
