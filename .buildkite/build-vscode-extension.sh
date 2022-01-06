#!/bin/bash

set -euo pipefail

pushd vscode_extension
yarn
yarn generate-package
popd

rm -rf _out_
mkdir -p _out_/vscode_extension
cp vscode_extension/sorbet.vsix _out_/vscode_extension/sorbet.vsix