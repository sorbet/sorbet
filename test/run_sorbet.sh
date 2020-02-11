#!/bin/bash

set -euo pipefail

pushd "$(dirname "$0")/.." > /dev/null

source "test/logging.sh"

debug=
if [ "$1" == "-d" ]; then
  debug=1
  shift 1
fi

if [ -z "$*" ]; then
  echo "Usage: test/run_sorbet.sh [-d] <test_file>"
  exit 1
fi

if [ -z "${llvmir:-}" ]; then
  llvmir=$(mktemp -d)
fi

echo
info "Building SorbetLLVM..."
if [ -n "$debug" ]; then
  bazel build //main:sorbet --config dbg
else
  bazel build //main:sorbet -c opt
fi

if [ -z "$debug" ]; then
  command=( "bazel-bin/main/sorbet" )
else
  command=( "lldb" "--" "./bazel-bin/main/sorbet" )
fi

command=( "${command[@]}" \
  --silence-dev-message \
  "--llvm-ir-folder=$llvmir" \
  "$@"  \
)

echo
info "Running SorbetLLVM to generate LLVM + shared object..."
info "├─ ${command[*]}"

if "${command[@]}"; then
  success "└─ successfully generated LLVM output."
else
  fatal "└─ compiling to LLVM failed. See above."
fi
