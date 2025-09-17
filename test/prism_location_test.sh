#!/bin/bash

set -euo pipefail

# Set up the runfiles
# shellcheck disable=SC1090
source "${RUNFILES_DIR:-/dev/null}/bazel_tools/tools/bash/runfiles/runfiles.bash"
if [[ ! -d "${RUNFILES_DIR:-/dev/null}" ]]; then
  echo >&2 "ERROR: could not find runfiles directory"
  exit 1
fi

# Locate the Sorbet binary
SORBET_BIN="$(rlocation com_stripe_ruby_typer/main/sorbet)"
root_dir="$(rlocation com_stripe_ruby_typer)"

# The Ruby file to test is passed as the first argument
test_file="test/$1"

# Temporary files to hold parser outputs
sorbet_output_sorbet=$(mktemp -t "parser_original")
sorbet_output_prism=$(mktemp  -t "parser_prism   ")

(
  cd "$root_dir"

  set +e  # Temporarily disable exit on error
  # Run Sorbet with default parser
  "$SORBET_BIN" --print=parse-tree-json-with-locs --parser=original "$test_file" > "$sorbet_output_sorbet" 2>/dev/null
  # Run Sorbet with Prism parser
  "$SORBET_BIN" --print=parse-tree-json-with-locs --parser=prism "$test_file" > "$sorbet_output_prism" 2>/dev/null
  set -e  # Re-enable exit on error
)

# Compare outputs
if ! diff -q "$sorbet_output_prism" "$sorbet_output_sorbet" >/dev/null 2>&1; then
    echo "Running sorbet --print=parse-tree-json-with-locs produced different results for $test_file"
    echo
    (
      cd "$(dirname "$sorbet_output_sorbet")"
      diff -u "$(basename "${sorbet_output_sorbet}")" "$(basename "$sorbet_output_prism")"
    )
    rm "$sorbet_output_prism" "$sorbet_output_sorbet"
    exit 1
fi

# Clean up temporary files
rm "$sorbet_output_prism" "$sorbet_output_sorbet"
echo "Test passed for $test_file"
