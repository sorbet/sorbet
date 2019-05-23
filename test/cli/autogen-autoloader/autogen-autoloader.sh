#!/bin/bash
set -eu

rm -rf output
mkdir -p output

main/sorbet --silence-dev-message --stop-after=namer -p autogen-autoloader:output \
  test/cli/autogen-autoloader/{foo,bar,bar2,errors}.rb \
  test/cli/autogen-autoloader/scripts/baz.rb 2>&1

for file in $(find output -type f | sort); do
  printf "\n--- %s\n" "$file"
  cat "$file"
done
