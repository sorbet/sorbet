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


if [[ "linux" == "$platform" ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-linux"
elif [[ "mac" == "$platform" ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-mac"
fi

export JOB_NAME=test-static-sanitized
source .buildkite/tools/setup-bazel.sh

echo will run with $CONFIG_OPTS

err=0

mkdir -p _out_

# NOTE: we skip the compiler tests because llvm doesn't interact well with the sanitizer
./bazel test \
  --experimental_generate_json_trace_profile --profile=_out_/profile.json \
  --test_tag_filters=-compiler \
  --build_tag_filters=-compiler \
  --build_tests_only \
  @gems//... \
  //gems/sorbet/test/snapshot \
  //gems/sorbet/test/hidden-method-finder \
  //... $CONFIG_OPTS --test_summary=terse || err=$?


# --- post process test results here if you want ---

if [ "$err" -ne 0 ]; then
    exit "$err"
fi
