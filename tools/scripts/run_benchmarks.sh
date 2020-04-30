#!/bin/bash

set -euo pipefail

discovery=1
if [[ $# -eq 1 ]]; then
  input=$(realpath "$1")
  discovery=""
fi

cd "$(dirname "$0")"
cd ../..
# we are now at the repo root.

rm -rf tmp/bench
mkdir -p tmp/bench

# TODO(jez) Careful! These must use the same configuration!
# (Alternatively: be sure to compile the right one right before it's used.)
bazel build //main:sorbet -c opt
bazel run @sorbet_ruby//:ruby -c opt -- --version

paths=(test/testdata/ruby_benchmark)

rb_src=()
if [[ "1" == "$discovery" ]]; then
  while IFS='' read -r line; do
      rb_src+=("$line")
  done < <(find "${paths[@]}" -name '*.rb' | grep -v disabled | sort)
else
  rb_src=("$input")
fi

pushd tmp/bench &>/dev/null
    (time for _ in {1..10}; do ../../bazel-bin/external/sorbet_ruby/toolchain/bin/ruby -r ../../bazel-sorbet_llvm/external/com_stripe_ruby_typer/gems/sorbet-runtime/lib/sorbet-runtime.rb -e 1 --disable=gems --disable=did_you_mean ; done) 2>&1|grep real | cut -d$'\t' -f 2 > baseline_time
    minutes_baseline=$(cut -d "m" -f1 < baseline_time)
    seconds_baseline=$(cut -d "m" -f2 < baseline_time | cut -d "s" -f 1)
    baseline_time=$(echo "scale=3;(${minutes_baseline} * 60 + ${seconds_baseline})/10"| bc)
    echo -e "ruby vm startup time:\t$baseline_time"
popd &>/dev/null


echo -e "source\tinterpreted\tcompiled"
for this_src in "${rb_src[@]}"; do
    rm tmp/bench/*
    cp "$this_src" tmp/bench/target.rb
    echo -en "${this_src#test/testdata/ruby_benchmark/}\t"
    llvmir=. test/run_sorbet.sh tmp/bench/target.rb &>/dev/null
    pushd tmp/bench &>/dev/null
    (time for _ in {1..10}; do ../../bazel-bin/external/sorbet_ruby/toolchain/bin/ruby -r ../../bazel-sorbet_llvm/external/com_stripe_ruby_typer/gems/sorbet-runtime/lib/sorbet-runtime.rb ./target.rb --disable=gems --disable=did_you_mean ; done) 2>&1|grep real | cut -d$'\t' -f 2 > ruby_runtime

    minutes_ruby=$(cut -d "m" -f1 < ruby_runtime)
    seconds_ruby=$(cut -d "m" -f2 < ruby_runtime | cut -d "s" -f 1)
    ruby_time=$(echo "scale=3;(${minutes_ruby} * 60 + ${seconds_ruby})/10"| bc)
    echo -en "$ruby_time\t"

    (time for _ in {1..10}; do ../../bazel-bin/external/sorbet_ruby/toolchain/bin/ruby -r ../../bazel-sorbet_llvm/external/com_stripe_ruby_typer/gems/sorbet-runtime/lib/sorbet-runtime.rb ../../test/patch_require.rb -e "require './target.rb.so'" --disable=gems --disable=did_you_mean ; done) 2>&1|grep real | cut -d$'\t' -f 2 > compiled_runtime
    minutes_compiled=$(cut -d "m" -f1 < compiled_runtime)
    seconds_compiled=$(cut -d "m" -f2 < compiled_runtime | cut -d "s" -f 1)
    compiled_time=$(echo "scale=3;(${minutes_compiled} * 60 + ${seconds_compiled})/10"| bc)
    echo -e "$compiled_time"
    popd &>/dev/null
done

