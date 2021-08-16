#!/bin/bash
set -eu

preamble="# frozen_string_literal: true
# typed: true
"

rm -rf output
mkdir -p output
echo "TODO: fix this test once autogen-autoloader-packaged is supported"

main/sorbet --silence-dev-message --stop-after=namer \
  --stripe-packages \
  -p autogen-autoloader:output \
  --autogen-autoloader-modules=Project \
  --autogen-autoloader-ignore=scripts/ \
  --autogen-autoloader-preamble "$preamble" \
  --autogen-autoloader-packaged \
  test/cli/autogen-multipkg-autoloader/foo/*.rb \
  test/cli/autogen-multipkg-autoloader/bar/*.rb 2>&1

for file in $(find output -type f | sort); do
  printf "\n--- %s\n" "$file"
  cat "$file"
done
