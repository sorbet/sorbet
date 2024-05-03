#!/bin/bash

set -euo pipefail

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

export JOB_NAME=test-rbi-gen
source .buildkite/tools/setup-bazel.sh

err=0

echo "+++ running tests"

# `-c opt` is required, otherwise the tests are too slow
# forcedebug is really the ~only thing in `--config=dbg` we care about.
# must come after `-c opt` because `-c opt` will define NDEBUG on its own
test_args=(
  "//test:end_to_end_rbi_test"
  "//test:single_package_runner"
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

  buildkite-agent annotate --context "test-rbi-gen.sh" --style error --append < "$failing_tests"

  exit "$err"
fi
