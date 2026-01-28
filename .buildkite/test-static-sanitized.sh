#!/bin/bash

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

test_args=()

if [[ "linux" == "$platform" ]]; then
  test_args+=("--config=buildfarm-sanitized-linux")
elif [[ "mac" == "$platform" ]]; then
  test_args+=("--config=buildfarm-sanitized-mac")
fi

export JOB_NAME=test-static-sanitized
source .buildkite/tools/setup-bazel.sh

echo -- will run with "${test_args[@]}"

err=0

mkdir -p _out_

test_args+=(
  "--build_tests_only"
  "//..."
)

./bazel test \
  --experimental_generate_json_trace_profile \
  --profile=_out_/profile.json \
  --test_summary=terse \
  "${test_args[@]}" || err=$?

if [ "$err" -ne 0 ]; then
  echo "--- annotating build result"
  failing_tests="$(mktemp)"

  echo 'Run this command to run failing tests locally:' >> "$failing_tests"
  echo >> "$failing_tests"
  echo '```bash' >> "$failing_tests"

  # Take the lines that start with target labels.
  # Lines look like "//foo  FAILED in 10s"
  failing_targets=$(
    { ./bazel test --test_summary=terse "${test_args[@]}" || true ; } | \
      grep '^//' | \
      sed -e 's/ .*//'
  )

  num_failing=$(echo "$failing_targets" | wc -l | tr -d ' ')

  # Use --config=dbg instead of sanitized because it's more likely that they've
  # already built this config locally, and most test failures reproduce outside
  # of sanitized mode anyways.
  if [ "$num_failing" -eq 1 ]; then
    # Single test - put everything on one line
    echo "./bazel test --config=dbg --test_output=errors $failing_targets" >> "$failing_tests"
  else
    # Multiple tests - use multi-line format
    echo "./bazel test --config=dbg --test_output=errors \\" >> "$failing_tests"
    # Add backslash to all but the last line, indent all lines
    echo "$failing_targets" | sed -e '$!s/$/ \\/' -e 's/^/  /' >> "$failing_tests"
  fi

  echo '```' >> "$failing_tests"

  buildkite-agent annotate --context "test-static-sanitized.sh" --style error --append < "$failing_tests"

  exit "$err"
fi
