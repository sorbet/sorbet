#!/bin/bash

set -euo pipefail

base="$( cd "$(dirname "$0")" ; pwd -P )"/..

DEBUG=
if [ "$1" == "-d" ]; then
  DEBUG=1
  shift 1
fi

rb=$1

if [ -z "$rb" ]; then
  echo "Usage: test/run_ruby.sh <test_file>"
  exit 1
fi

pushd "$base" > /dev/null

ruby="./bazel-bin/external/ruby_2_6_3/ruby.runfiles/ruby_2_6_3"
ruby_bin="${ruby}/bin"
ruby_lib="${ruby}/lib/ruby/2.6.0"

# work-around for not being able to declare an empty array in bash-3.x
if [ -z "$DEBUG" ]; then
  command=( "${ruby_bin}/ruby" )
else
  command=("lldb" "--" "${ruby_bin}/ruby")
fi

if [ ! -f "${ruby_bin}/ruby" ]; then
  bazel build @ruby_2_6_3//:ruby
fi

command=("${command[@]}" \
  -W0 \
  -I "${ruby_lib}" -I "${ruby_lib}/x86_64-darwin18" \
  -I "$base/run/tools" -rpreamble.rb \
  "$base/$1" \
  )

"${command[@]}"

popd > /dev/null
