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

basename=
srcs=()
exp_extensions="llo ll stderr"
echo -e "source\tinterpreted\tcompiled"
for this_src in "${rb_src[@]}" DUMMY; do
    cp "$this_src" tmp/bench/target.rb
    pushd tmp/bench &>/dev/null
      ../../run/compile . target.rb &>/dev/null
      (time for i in {1..10}; do ../../bazel-bin/external/ruby_2_6_3/ruby -r ../../run/tools/preamble.rb ./target.rb; done) 2>&1|grep real | cut -d$'\t' -f 2 > ruby_runtime
      (time for i in {1..10}; do ../../bazel-bin/external/ruby_2_6_3/ruby -r ../../run/tools/preamble.rb -e "require './target.so'" ; done) 2>&1|grep real | cut -d$'\t' -f 2 > compiled_runtime

      minutes_ruby=$(cat ruby_runtime|cut -d "m" -f1)
      seconds_ruby=$(cat ruby_runtime|cut -d "m" -f2 |cut -d "s" -f 1)
      ruby_time=$(echo "scale=3;(${minutes_ruby} * 60 + ${seconds_ruby})/10"| bc)

      minutes_compiled=$(cat compiled_runtime|cut -d "m" -f1)
      seconds_compiled=$(cat compiled_runtime|cut -d "m" -f2 |cut -d "s" -f 1)
      compiled_time=$(echo "scale=3;(${minutes_compiled} * 60 + ${seconds_compiled})/10"| bc)

      echo -e "${this_src#test/testdata/ruby_benchmark/}\t$ruby_time\t$compiled_time"
      rm *
    popd &>/dev/null
done

