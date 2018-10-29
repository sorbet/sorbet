#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

bazel build //emscripten:sorbet-wasm.tar --config=webasm-darwin

echo
echo "./bazel-bin/emscripten:sorbet-wasm.tar should contain \".wasm\" and \".js\" file"
echo "Please copy them to your clone of sorbet.run repo and make a commit"
echo "     tar -C ../sorbet.run/docs/ -xvf ./bazel-bin/emscripten/sorbet-wasm.tar sorbet-wasm.wasm sorbet-wasm.js"