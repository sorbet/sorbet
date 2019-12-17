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
  -I "run/tools" -rpreamble.rb \
  -e "require './$rb'" \
  "$@" \
  )

"${command[@]}"

popd > /dev/null
