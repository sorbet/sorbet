#!/bin/bash

set -euo pipefail

# shellcheck source-path=SCRIPTDIR/..
source .buildkite/tools/with_backoff.sh

pushd vscode_extension

# Sometimes requests to https://registry.yarnpkg.com hit transient errors.
with_backoff yarn

xvfb-run -a yarn test

popd
