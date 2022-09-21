#!/bin/bash

set -euo pipefail

pushd vscode_extension

source .buildkite/tools/with_backoff.sh

# Sometimes requests to https://registry.yarnpkg.com hit transient errors.
with_backoff yarn

yarn generate-package

popd

rm -rf _out_
mkdir -p _out_/vscode_extension
cp vscode_extension/sorbet.vsix _out_/vscode_extension/sorbet.vsix
