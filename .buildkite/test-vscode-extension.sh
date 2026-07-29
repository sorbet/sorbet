#!/bin/bash

set -euo pipefail

if [ "${BUILDKITE_BRANCH:-}" != "master" ]; then
  if ! git diff --name-only "origin/master...HEAD" | grep -q '^vscode_extension/'; then
    echo "Skipping because there are no changes to vscode_extension/"
    exit 0
  fi
fi

# shellcheck source-path=SCRIPTDIR/..
source .buildkite/tools/with_backoff.sh

pushd vscode_extension

# Sometimes requests to https://registry.yarnpkg.com hit transient errors.
with_backoff yarn

xvfb-run -a yarn test

popd
