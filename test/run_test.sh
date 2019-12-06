#!/bin/bash

set -euo pipefail

base="$( cd "$(dirname "$0")" ; pwd -P )"/..

rb=$1

if [ -z "$rb" ]; then
  echo "Usage: test/run_test.sh <test_file>"
  exit 1
fi

pushd "$base" > /dev/null

if [ ! -f ./bazel-bin/external/ruby_2_6_3/ruby ]; then
  bazel build @ruby_2_6_3//:ruby
fi

./bazel-bin/external/ruby_2_6_3/ruby -I "$base/run/tools" -rpreamble.rb "$base/$1"

popd > /dev/null
