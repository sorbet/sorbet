#!/bin/bash

set -euo pipefail

pushd "$(dirname "$0")/.." > /dev/null

DEBUG=
if [ "$1" == "-d" ]; then
  DEBUG=1
  shift 1
fi

rb=$1
shift

if [ -z "$rb" ]; then
  echo "Usage: test/run_ruby.sh <test_file>"
  exit 1
fi

ruby="./bazel-bin/external/sorbet_ruby/ruby"

if [ -n "$DEBUG" ]; then
  bazel build @sorbet_ruby//:ruby --config dbg --config static-libs 2>/dev/null
  command=("lldb" "--" "${ruby}")
else
  bazel build @sorbet_ruby//:ruby -c opt 2>/dev/null
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
  -e "require './$rb'" \
  "$@" \
  )

llvmir="$llvmir" "${command[@]}"
exit_code=$?

popd > /dev/null

exit $exit_code
