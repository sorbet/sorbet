#!/bin/bash

# This script runs memory comparisons for the Prism parser in the Sorbet type checker.
# It assumes the presence of a few directories and tools:
# - The yjit-bench directory, which contains the benchmarks for YJIT (https://github.com/Shopify/yjit-bench)
# - The Shopify directory, which contains the Shopify codebase (https://github.com/Shopify/shopify)
# - gtime, a command-line time and memory analysis tool (https://www.gnu.org/software/time/)

# ----- Setup -----

YJIT_BENCH_DIR="../yjit-bench"
SHOPIFY_DIR="../../Shopify/shopify"
RBI_DIR="../../Shopify/rbi"

export SORBET_SILENCE_DEV_MESSAGE=1

# Check if required directories exist
for dir in "$YJIT_BENCH_DIR" "$SHOPIFY_DIR" "$RBI_DIR"; do
    if [ ! -d "$dir" ]; then
        echo "Please clone the required directories before running this script."
        exit 1
    fi
done

# Check and install gtime if necessary
if ! command -v gtime &> /dev/null; then
    echo "Installing gtime..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        brew install gnu-time
    else
        echo "Please install gtime: https://www.gnu.org/software/time/"
        exit 1
    fi
fi

output_file_name="$(date '+%Y-%m-%d')-$(git rev-parse --short HEAD).txt"
parsers=("prism" "sorbet")

# ----- Run Benchmarks -----

run_benchmark() {
    local title="$1"
    local output_dir="$2"
    local command="$3"

    echo "#### $title ####"
    output_file="prism_benchmarks/memory/data/$output_dir/${output_file_name}"

    for parser in "${parsers[@]}"; do
        gtime --verbose --output="$output_file" --append \
            bazel-bin/main/sorbet --parser=$parser $command &> /dev/null
        echo "" >> "$output_file"
        # Calculate peak memory usage:
        # - Search for lines that contain peak memory usage info
        # - Take the last one
        # - Extract the memory usage value
        # - Convert it to MB
        peak_memory=$(grep "Maximum resident set size (kbytes):" "$output_file" | tail -n 1 | awk '{print $6/1024 " MB"}')
        echo "-> Peak Memory Usage with $parser parser: $peak_memory"
    done
    echo ""
}

run_benchmark "Memory Check 1: yjit-bench, parser only" "parser/yjit-bench" "--stop-after=parser $YJIT_BENCH_DIR/benchmarks"
run_benchmark "Memory Check 2: shopify, parser only" "parser/shopify" "--stop-after=parser $SHOPIFY_DIR"
run_benchmark "Memory Check 3: prism regression tests, whole pipeline" "pipeline/prism_regression" "test/prism_regression"
run_benchmark "Memory Check 4: RBI gem, whole pipeline" "pipeline/rbi" $RBI_DIR
