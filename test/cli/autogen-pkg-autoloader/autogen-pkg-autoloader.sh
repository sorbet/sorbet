#!/bin/bash
set -eu

preamble="# frozen_string_literal: true
# typed: true
"

rm -rf output
mkdir -p output

main/sorbet --silence-dev-message --stop-after=namer \
  --stripe-packages \
  -p autogen-autoloader:output \
  --autogen-autoloader-modules={Foo,Yabba} \
  --autogen-autoloader-exclude-require=byebug \
  --autogen-autoloader-ignore=scripts/ \
  --autogen-autoloader-preamble "$preamble" \
  test/cli/autogen-pkg-autoloader/{foo,bar,bar2,errors,__package}.rb \
  test/cli/autogen-pkg-autoloader/scripts/baz.rb 2>&1

for file in $(find output -type f | sort); do
  printf "\n--- %s\n" "$file"
  cat "$file"
done
