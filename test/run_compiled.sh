#!/bin/bash

set -euo pipefail

pushd "$(dirname "$0")/.." > /dev/null

source "test/logging.sh"

DEBUG=
if [ "$1" == "-d" ]; then
  DEBUG=1
  shift 1
fi

rb=$1
shift

if [ -z "$rb" ]; then
  echo "Usage: test/run_compiled.sh [-d] <test_file>"
  echo
  echo "  NOTE: if the 'llvmir' environmenet variable is set, that will be used"
  echo "        for compiler output instead."
  exit 1
fi

# Export llvmir so that run_sorbet picks it up. Real argument parsing in
# run_sorbet.sh would probably be better.
llvmir="$(mktemp -d)"
export llvmir

# ensure that the extension is built
"test/run_sorbet.sh" "$rb"

ruby="./bazel-bin/external/sorbet_ruby/toolchain/bin/ruby"

if [ -n "$DEBUG" ]; then
  bazel build @sorbet_ruby//:ruby --config dbg 2>/dev/null
  command=("lldb" "--" "${ruby}")
else
  bazel build @sorbet_ruby//:ruby -c opt 2>/dev/null
  command=( "${ruby}" )
fi

command=("${command[@]}" \
  "--disable=gems" \
  "--disable=did_you_mean" \
  -I "run/tools" -rpreamble.rb -rpatch_require.rb \
  -e "require './$rb'" \
  "$@" \
  )

# Use force_compile to make patch_require.rb fail if the compiled extension
# isn't found.
llvmir="$llvmir" force_compile=1 "${command[@]}"
exit_code=$?

popd > /dev/null

exit $exit_code
