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

yarn generate-package

popd

rm -rf _out_
mkdir -p _out_/vscode_extension
cp vscode_extension/sorbet.vsix _out_/vscode_extension/sorbet.vsix
