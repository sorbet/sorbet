#!/bin/bash

set -euo pipefail

base="$( cd "$(dirname "$0")" ; pwd -P )"/..

pushd "$base" > /dev/null

source "test/logging.sh"

DEBUG=
if [ "$1" == "-d" ]; then
  DEBUG=1
  shift 1
fi

rb=$1

if [ -z "$rb" ]; then
  echo "Usage: test/run_sorbet.sh [-d] <test_file>"
  exit 1
fi

llvmir=$(mktemp -d)

if [ ! -f ./bazel-bin/main/sorbet ]; then
  bazel build //main:sorbet
fi

if [ -z "$DEBUG" ]; then
  info "Running sorbet"
  command=( "./bazel-bin/main/sorbet" )
else
  attn "Writing output to '$llvmir'"
  info "Running sorbet (under lldb)"

  command=( "lldb" "--" "./bazel-bin/main/sorbet" )
fi

command=( "${command[@]}" \
  --silence-dev-message \
  "--llvm-ir-folder=$llvmir" \
  --force-compiled \
  "$rb"  \
)

"${command[@]}"

popd > /dev/null

if [ -z "$DEBUG" ]; then
  rm -r "$llvmir"
fi
