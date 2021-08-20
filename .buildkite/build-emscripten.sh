#!/bin/bash

set -euo pipefail

echo "--- Pre-setup"

command -v realpath

export JOB_NAME=build-emscripten
source .buildkite/tools/setup-bazel.sh

PATH=$PATH:$(pwd)
export PATH

./bazel build //emscripten:sorbet-wasm.tar --config=webasm-linux --strip=always

rm -rf _out_
mkdir -p _out_/webasm
cp bazel-bin/emscripten/sorbet-wasm.tar _out_/webasm/sorbet-wasm.tar
