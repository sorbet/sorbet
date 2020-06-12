#!/bin/bash
set -eu

preamble="# frozen_string_literal: true
# typed: true
"

rm -rf output
mkdir -p output

main/sorbet --silence-dev-message \
  --stripe-packages \
  --stop-after=namer \
  -p autogen-autoloader:output \
  --autogen-autoloader-modules={Foo,A,PkA} \
  --autogen-autoloader-preamble "$preamble" \
  test/cli/package-autoloader/*.rb \
  test/cli/package-autoloader/**/*.rb \
  2>&1

tree output # TODO

for file in $(find output -type f | sort); do
  printf "\n--- %s\n" "$file"
  cat "$file"
done
