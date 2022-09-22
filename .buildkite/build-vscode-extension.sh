#!/bin/bash

set -euo pipefail

source .buildkite/tools/with_backoff.sh

pushd vscode_extension

# Sometimes requests to https://registry.yarnpkg.com hit transient errors.
with_backoff yarn

yarn generate-package

popd

rm -rf _out_
mkdir -p _out_/vscode_extension
cp vscode_extension/sorbet.vsix _out_/vscode_extension/sorbet.vsix
