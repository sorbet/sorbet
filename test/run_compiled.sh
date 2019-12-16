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

ruby="./bazel-bin/external/ruby_2_6_3/ruby.runfiles/ruby_2_6_3"
ruby_bin="${ruby}/bin"
ruby_lib="${ruby}/lib/ruby/2.6.0"

# work-around for not being able to declare an empty array in bash-3.x
if [ -z "$DEBUG" ]; then
  command=( "${ruby_bin}/ruby" )
else
  command=("lldb" "--" "${ruby_bin}/ruby")
fi

if [ -n "$DEBUG" ]; then
  bazel build @ruby_2_6_3//:ruby --config dbg --config static-libs 2>/dev/null
else
  bazel build @ruby_2_6_3//:ruby 2>/dev/null
fi

command=("${command[@]}" \
  -W0 \
  -I "${ruby_lib}" -I "${ruby_lib}/x86_64-darwin18" \
  -I "run/tools" -rpreamble.rb -rpatch_require.rb \
  -e "require './$rb'" \
  )

# Use force_compile to make patch_require.rb fail if the compiled extension
# isn't found.
llvmir="$llvmir" force_compile=1 "${command[@]}"

popd > /dev/null
