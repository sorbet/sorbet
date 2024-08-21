#!/bin/bash

# This script runs the benchmarks for the Prism parser in the Sorbet type checker.
# It assumes the presence of a few directories and tools:
# - The yjit-bench directory, which contains the benchmarks for YJIT (https://github.com/Shopify/yjit-bench)
# - Hyperfine, a command-line benchmarking tool (https://github.com/sharkdp/hyperfine)

# Check if the yjit-bench directory exists
if [ ! -d "../yjit-bench" ]; then
  echo "Please clone the yjit-bench directory before running this script: https://github.com/Shopify/yjit-bench"
  exit 1
fi

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

file_name="$(date '+%Y-%m-%d')-$(git rev-parse --short HEAD).json"

echo "#### Benchmark 1: yjit-bench, parser only"

# Run the parser benchmarks
# These benchmarks compare the performance of the Prism parser with the Sorbet parser without
# any other parts of the type checker enabled.
hyperfine \
  --warmup=10 --export-json="prism_benchmarks/time/data/parser/yjit-bench/$file_name" --parameter-list parser sorbet,prism \
  "bazel-bin/main/sorbet --parser={parser} --stop-after=parser ../yjit-bench/benchmarks"

echo "#### Benchmark 2: sorbet-test, parser only"

# Benchmark against sorbet tests to capture performance around parser errors
# Pass the --ignore-failure flag to prevent hyperfine from exiting on parser errors
hyperfine \
  --warmup=10 --export-json="prism_benchmarks/time/data/parser/sorbet-tests/$file_name" --parameter-list parser sorbet,prism \
  "bazel-bin/main/sorbet --parser={parser} --stop-after=parser test/testdata" --ignore-failure

echo "#### Benchmark 3: prism regression tests, whole pipeline"

# Run the pipeline benchmarks
# These benchmarks compare the performance of the entire Sorbet type checking pipeline with
# the Prism parser enabled and the Sorbet parser enabled. These benchmarks must be run on a
# smaller set of files that only contain nodes supported by the Prism --> Sorbet translation layer.
hyperfine \
  --warmup=10 --export-json="prism_benchmarks/time/data/pipeline/$file_name" --parameter-list parser sorbet,prism \
  "bazel-bin/main/sorbet --parser={parser} test/prism_regression"
