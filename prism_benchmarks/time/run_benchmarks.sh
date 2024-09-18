#!/bin/bash

# This script runs the benchmarks for the Prism parser in the Sorbet type checker.
# It assumes the presence of a few directories and tools:
# - The yjit-bench directory, which contains the benchmarks for YJIT (https://github.com/Shopify/yjit-bench)
# - The rbi directory, which contains the RBI gem (https://github.com/Shopify/rbi)
# - Hyperfine, a command-line benchmarking tool (https://github.com/sharkdp/hyperfine)

# ----- Setup -----

YJIT_BENCH_DIR="../yjit-bench"
RBI_DIR="../rbi"

export SORBET_SILENCE_DEV_MESSAGE=1

# Check if required directories exist
for dir in "$YJIT_BENCH_DIR" "$RBI_DIR"; do
  if [ ! -d "$dir" ]; then
    echo "Please clone the required directories before running this script."
    exit 1
  fi
done

# Check if hyperfine is installed
if [! command -v hyperfine >/dev/null 2>&1 ]; then
  echo "Please install hyperfine before running this script: https://github.com/sharkdp/hyperfine"
  exit 1
fi

# Enable globstar for recursive directory listing
# This is for Bash 4.0 and later, which does not have recursive globbing enabled by default
shopt -s globstar

# Read all files in the benchmarks directory to warm up the file system cache
for i in {1..10}; do
  cat ../yjit-bench/benchmarks/**/*.rb > /dev/null
  cat test/testdata/**/*.rb > /dev/null
done

output_file_name="$(date '+%Y-%m-%d')-$(git rev-parse --short HEAD).json"

# ----- Run Benchmarks -----

run_benchmark() {
  local title="$1"
  local output_dir="$2"
  local command="$3"

  echo "#### $title ####"
  output_file="prism_benchmarks/time/data/$output_dir/${output_file_name}"

  hyperfine \
    --warmup=10 --export-json="$output_file" --parameter-list parser sorbet,prism \
    "bazel-bin/main/sorbet --parser={parser} $command" --ignore-failure
}

run_benchmark "Benchmark 1: yjit-bench, parser only" "parser/yjit-bench" "--stop-after=parser $YJIT_BENCH_DIR/benchmarks"
run_benchmark "Benchmark 2: sorbet-test, parser only" "parser/sorbet-tests" "--stop-after=parser test/testdata"
run_benchmark "Benchmark 3: prism regression tests, whole pipeline" "pipeline/prism_regression" "test/prism_regression"
run_benchmark "Benchmark 4: RBI gem, whole pipeline" "pipeline/rbi" "$RBI_DIR"
