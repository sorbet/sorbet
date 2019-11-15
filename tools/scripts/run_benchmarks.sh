#!/bin/bash

set -euo pipefail

cd "$(dirname "$0")"
cd ../..
# we are now at the repo root.

rm -rf tmp/bench
mkdir -p tmp/bench

bazel build //main:sorbet -c opt
bazel run @ruby_2_6_3//:ruby -c opt -- --version

paths=(test/testdata/ruby_benchmark)

rb_src=()
while IFS='' read -r line; do
    rb_src+=("$line")
done < <(find "${paths[@]}" -name '*.rb' | grep -v disabled | sort)

echo -e "source\tinterpreted\tcompiled"
for this_src in "${rb_src[@]}" DUMMY; do
    cp "$this_src" tmp/bench/target.rb
    pushd tmp/bench &>/dev/null
      echo -en "${this_src#test/testdata/ruby_benchmark/}\t"
      ../../run/compile . target.rb &>/dev/null
      (time for _ in {1..10}; do ../../bazel-bin/external/ruby_2_6_3/ruby -r ../../run/tools/preamble.rb ./target.rb --disable=gems --disable=did_you_mean ; done) 2>&1|grep real | cut -d$'\t' -f 2 > ruby_runtime

      minutes_ruby=$(cut -d "m" -f1 < ruby_runtime)
      seconds_ruby=$(cut -d "m" -f2 < ruby_runtime | cut -d "s" -f 1)
      ruby_time=$(echo "scale=3;(${minutes_ruby} * 60 + ${seconds_ruby})/10"| bc)
      echo -en "$ruby_time\t"

      (time for _ in {1..10}; do ../../bazel-bin/external/ruby_2_6_3/ruby -r ../../run/tools/preamble.rb -e "require './target.rb.so'" --disable=gems --disable=did_you_mean ; done) 2>&1|grep real | cut -d$'\t' -f 2 > compiled_runtime
      minutes_compiled=$(cut -d "m" -f1 < compiled_runtime)
      seconds_compiled=$(cut -d "m" -f2 < compiled_runtime | cut -d "s" -f 1)
      compiled_time=$(echo "scale=3;(${minutes_compiled} * 60 + ${seconds_compiled})/10"| bc)
      echo -e "$compiled_time"

      rm ./*
    popd &>/dev/null
done

