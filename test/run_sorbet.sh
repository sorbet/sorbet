#!/bin/bash

set -euo pipefail

base="$( cd "$(dirname "$0")" ; pwd -P )"/..

rb=$1

if [ -z "$rb" ]; then
  echo "Usage: test/run_sorbet.sh <test_file>"
  exit 1
fi


mkdir -p llvmir
output=$PWD/llvmir

pushd "$base" > /dev/null
if [ ! -f ./bazel-bin/main/sorbet ]; then
  bazel build //main:sorbet
fi

./bazel-bin/main/sorbet --silence-dev-message --llvm-ir-folder="$output" --force-compiled "$rb"
popd > /dev/null

echo "Output written to 'llvmir'"
