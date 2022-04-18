#!/usr/bin/env bash

set -euo pipefail

if [ "${SIMULATE_FAIL:-}" != "" ]; then
  echo "Simulating build failure. Exiting status 1"
  exit 1
fi

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac


if [[ "linux" == "$platform" ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-linux"
elif [[ "mac" == "$platform" ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-mac"
fi

export JOB_NAME=test-sorbet-rubyfmt
source .buildkite/tools/setup-bazel.sh

echo will run with $CONFIG_OPTS

err=0

# NOTE: we skip the compiler tests because llvm doesn't interact well with the sanitizer
test_args=(
  "$CONFIG_OPTS"
  "--config=rubyfmt"
  //main:sorbet
  //experiments/rubyfmt:rubyfmt_test
)

./bazel test \
  --test_summary=terse \
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
  #
  # Use --config=dbg instead of sanitized because it's more likely that they've
  # already built this config locally, and most test failures reproduce outside
  # of sanitized mode anyways.
  echo '  --config=dbg' >> "$failing_tests"
  echo '```' >> "$failing_tests"

  buildkite-agent annotate --context "test-sorbet-rubyfmt.sh" --style error --append < "$failing_tests"

  # NOTE: this job explicitly does not block the build
fi
