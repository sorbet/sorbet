#!/bin/bash

set -euo pipefail

echo "--- Pre-setup"

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == "$platform" ]]; then
  echo "only mac builds are supported"
  exit 1
elif [[ "mac" == "$platform" ]]; then
  echo "mac builds are supported"
  command -v node >/dev/null 2>&1 || brew install node
  command -v realpath >/dev/null 2>&1 || brew install realpath
fi

export JOB_NAME=build-emscripten
source .buildkite/tools/setup-bazel.sh

PATH=$PATH:$(pwd)
export PATH
tools/scripts/update-sorbet.run.sh

rm -rf _out_
mkdir -p _out_/webasm
cp bazel-bin/emscripten/sorbet-wasm.tar _out_/webasm/sorbet-wasm.tar
