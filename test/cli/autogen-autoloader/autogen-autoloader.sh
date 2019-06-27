#!/bin/bash
set -eu

preamble="# frozen_string_literal: true
# typed: true
"

rm -rf output
mkdir -p output

main/sorbet --silence-dev-message --stop-after=namer -p autogen-autoloader:output \
  --autogen-autoloader-modules={Foo,Yabba} \
  --autogen-autoloader-exclude-require=byebug \
  --autogen-autoloader-ignore=scripts/ \
  --autogen-autoloader-preamble "$preamble" \
  test/cli/autogen-autoloader/{foo,bar,bar2,errors}.rb \
  test/cli/autogen-autoloader/scripts/baz.rb 2>&1

for file in $(find output -type f | sort); do
  printf "\n--- %s\n" "$file"
  cat "$file"
done

echo
echo "--- missing output directory"
main/sorbet --silence-dev-message --stop-after=namer -p autogen-autoloader \
  test/cli/autogen-autoloader/scripts/baz.rb 2>&1
