#!/bin/bash

set -euo pipefail

usage() {
  cat <<EOF
Compare the performance of the compiler against the interpreter.

Usage:
  tools/scripts/run_benchmarks.sh [options] [[-b <baseline>.rb] -f <benchmark>.rb ...]

Options:
  -f <benchmark>.rb  Run benchmark specified in <benchmark>.rb via the
                     interpreter and via the compiled output. May be repeated
                     to run multiple benchmarks. If omitted, runs all files in
                     test/testdata/ruby_benchmark/**/*.rb, except those that
                     are disabled or too slow.

  -b <baseline>.rb   Run benchmark <baseline>.rb, and treat it like the
                     baseline (subtract its time from all other benchmarks)

  -d <delimiter>     Use <delimiter> to separate columns of output.
                     Suggestions are ',' or '|' or $'\\n'. [default: $'\\t']

  -v                 Verbose mode. Shows commands that are run, verbatim.
                     Useful for running the last (or only) benchmark elsewhere,
                     like under 'perf record --' or 'lldb --'.
                     (Looks best with with -d $'\\n', see above.)
EOF
}

verbose=
delim=$'\t'
baseline=""
benchmarks=()
while getopts 'vd:b:f:' optname; do
  case $optname in
    v)
      verbose=1
      ;;

    d)
      delim="$OPTARG"
      ;;

    b)
      # Used to subtract the "unimportant" parts out of the comparisons
      baseline="$OPTARG"
      ;;

    f)
      benchmarks+=( "$OPTARG" )
      ;;

    *)
      usage
      exit 1
      ;;
  esac
done

cd "$(dirname "$0")"
cd ../..
# we are now at the repo root.
repo_root="$PWD"

rm -rf tmp/bench
mkdir -p tmp/bench

# TODO(jez) Careful! These must use the same configuration!
# (Alternatively: be sure to compile the right one right before it's used.)
./bazel build //compiler:sorbet @sorbet_ruby_2_7_for_compiler//:ruby -c opt
./bazel run @sorbet_ruby_2_7_for_compiler//:ruby -c opt -- --version

if [ "${#benchmarks[@]}" -eq 0 ]; then
  if [ "$baseline" != "" ]; then
    echo "-b <baseline> is not supported with no -f <benchmark>"
    exit 1
  fi
  paths=(test/testdata/ruby_benchmark)

  while IFS='' read -r line; do
    benchmarks+=( "$line" )
  done < <(find "${paths[@]}" -name '*.rb' | grep -v -E '(disabled|too_slow)' | sort)
fi


ruby="${repo_root}/bazel-bin/external/sorbet_ruby_2_7_for_compiler/ruby"

command=()

set_startup() {
  unset llvmir
  command=(
    "${ruby}" \
      "--disable=gems" "--disable=did_you_mean" \
      -r "rubygems" \
      -r "${repo_root}/gems/sorbet-runtime/lib/sorbet-runtime.rb" \
      -e 1 \
  )
}

set_interpreted() {
  unset llvmir
  command=(
    "${ruby}" \
      "--disable=gems" "--disable=did_you_mean" \
      -r "rubygems" \
      -r "${repo_root}/gems/sorbet-runtime/lib/sorbet-runtime.rb" \
      ./target.rb \
  )
}

set_compiled() {
  export llvmir=.
  command=( \
    "${ruby}" \
      "--disable=gems" "--disable=did_you_mean" \
      -r "rubygems" \
      -r "${repo_root}/gems/sorbet-runtime/lib/sorbet-runtime.rb" \
      -r "${repo_root}/test/patch_require.rb" \
      -e "\$__sorbet_ruby_realpath='target.rb'; require './target.rb.so'" \
  )
}

measure() {
  pushd tmp/bench &>/dev/null
  (time for _ in {1..10}; do "${command[@]}"; done) 2>&1|grep real | cut -d$'\t' -f 2 > measurement
  minutes=$(cut -d "m" -f1 < measurement)
  seconds=$(cut -d "m" -f2 < measurement | cut -d "s" -f 1)
  popd &>/dev/null

  time="$(echo "scale=3;(${minutes} * 60 + ${seconds})/10" | bc)"
  echo -en "$time"
}

compile_benchmark() {
  benchmark="$1"

  rm tmp/bench/*
  cp "$benchmark" tmp/bench/target.rb
  llvmir=. compiled_out_dir=. test/run_sorbet.sh tmp/bench/target.rb &>/dev/null
}


# We require GNU printf for the fancy %q functionality.
__system_printf_has_q=
if env printf ' %q' 'test' &> /dev/null; then
  __system_printf_has_q=1
fi

echo_command() {
  # (The `env` avoids the bash built-in printf, which doesn't have %q)
  if [ "$__system_printf_has_q" != "" ]; then
    env printf ' %q' "${command[@]}"
  else
    # Won't be quoted, but at least it'll print something.
    # User will have to add their own quotes.
    env printf ' %s' "${command[@]}"
  fi
  printf '\n'
}

echo "ruby vm startup time: $(set_startup; measure)"

baseline_interpreted=
baseline_compiled=

echo -e "source${delim}interpreted${delim}compiled"
for benchmark in "$baseline" "${benchmarks[@]}"; do
  if [ "$benchmark" = "" ]; then
    # No baseline, skip this iteration
    continue
  fi


  echo -en "${benchmark#test/testdata/ruby_benchmark/}$delim"

  compile_benchmark "$benchmark"

  set_interpreted
  if [ -n "$verbose" ]; then
    >&2 printf '(in tmp/bench/) interpreted:'
    >&2 echo_command
  fi
  time_interpreted="$(measure)"
  echo -en "$time_interpreted$delim"
  if [ "$benchmark" = "$baseline" ]; then
    baseline_interpreted="$time_interpreted"
  fi

  set_compiled
  if [ -n "$verbose" ]; then
    >&2 printf '(in tmp/bench/) compiled: llvmir=.'
    >&2 echo_command
  fi
  time_compiled="$(measure)"
  echo "$time_compiled"
  if [ "$benchmark" = "$baseline" ]; then
    baseline_compiled="$time_compiled"
  fi

  if [ "$baseline" != "" ] && [ "$benchmark" != "$baseline" ]; then
    echo -en "${benchmark#test/testdata/ruby_benchmark/} - baseline$delim"
    echo -en "$(echo "$time_interpreted - $baseline_interpreted" | bc)$delim"
    echo "$time_compiled - $baseline_compiled" | bc
  fi

done

