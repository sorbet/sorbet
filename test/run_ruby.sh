#!/bin/bash

set -euo pipefail

pushd "$(dirname "$0")/.." > /dev/null

source "test/logging.sh"

debug=
if [ "$1" == "-d" ]; then
  debug=1
  shift 1
fi

rb_file=$1
shift

if [ -z "$rb_file" ]; then
  echo "Usage: test/run_ruby.sh <test_file>"



  exit 1
fi

ruby="./bazel-bin/external/sorbet_ruby/toolchain/bin/ruby"

echo
info "Building Ruby..."

if [ -n "$debug" ]; then
  bazel build @sorbet_ruby//:ruby --config dbg
  command=("lldb" "--" "${ruby}")
else
  bazel build @sorbet_ruby//:ruby -c opt
  command=( "${ruby}" )
fi

# Use a temp directory for LLVMIR so we don't accidentally pick up changes from
# the environment
llvmir=$(mktemp -d)
cleanup() {
    rm -r "$llvmir"
}
trap cleanup EXIT

command=("${command[@]}" \
  "--disable=gems" \
  "--disable=did_you_mean" \
  -I "run/tools" -rpreamble.rb -rpatch_require.rb \
  -e "require './$rb_file'" \
  "$@" \
  )

echo
info "Running compiled Ruby output..."
info "├─ ${command[*]}"

if llvmir="$llvmir" "${command[@]}"; then
  success "└─ done."
else
  fatal "└─ Non-zero exit. See above."
fi
