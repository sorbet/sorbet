#!/bin/bash

set -euo pipefail

whitelisted=0

echo "BUILDKITE_PULL_REQUEST=$BUILDKITE_PULL_REQUEST"
echo "BUILDKITE_PULL_REQUEST_REPO=$BUILDKITE_PULL_REQUEST_REPO"

if [[ "$BUILDKITE_PULL_REQUEST" == "false" ]]; then
  # whitelist commits that are triggered in branch builds of github.com/sorbet/sorbet
  echo "Automatically running build for non-PR commit pushed to sorbet/sorbet"
  whitelisted=1
fi

if [[ "$BUILDKITE_PULL_REQUEST_REPO" == "git://github.com/sorbet/sorbet.git" ]] ||
  [[ "$BUILDKITE_PULL_REQUEST_REPO" == "https://github.com/sorbet/sorbet.git" ]]; then
  # whitelist folks with write access to github.com/sorbet/sorbet
  echo "Automatically running build for non-fork PR created against sorbet/sorbet"
  whitelisted=1
fi

if [[ "${whitelisted}" -ne 1 ]] ; then
   (echo -e "steps:\\n  - block: \":key: Needs contributor approval!\"\\n  - wait: ~\\n";
    grep -v "steps:" .buildkite/pipeline.yaml ) | buildkite-agent pipeline upload
else
  buildkite-agent pipeline upload
fi
