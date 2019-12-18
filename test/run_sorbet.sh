#!/bin/bash

set -euo pipefail

pushd "$(dirname "$0")/.." > /dev/null

source "test/logging.sh"

DEBUG=
if [ "$1" == "-d" ]; then
  DEBUG=1
  shift 1
fi

if [ -z "$*" ]; then
  echo "Usage: test/run_sorbet.sh [-d] <test_file>"
  exit 1
fi

if [ -z "${llvmir:-}" ]; then
  llvmir=$(mktemp -d)
fi

attn "Writing output to '$llvmir'"

if [ -n "$DEBUG" ]; then
  bazel build //main:sorbet --config dbg --config static-libs 2>/dev/null
else
  bazel build //main:sorbet 2>/dev/null
fi

if [ -z "$DEBUG" ]; then
  info "Running sorbet"
  command=( "./bazel-bin/main/sorbet" )
else
  info "Running sorbet (under lldb)"

  command=( "lldb" "--" "./bazel-bin/main/sorbet" )
fi

command=( "${command[@]}" \
  --silence-dev-message \
  "--llvm-ir-folder=$llvmir" \
  --force-compiled \
  "$@"  \
)

"${command[@]}"

popd > /dev/null
