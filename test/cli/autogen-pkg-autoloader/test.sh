#!/bin/bash
set -eu

preamble="# frozen_string_literal: true
# typed: true
"

cwd="$(pwd)"
tmp=$(mktemp -d)
mkdir -p "$tmp/test/cli"
cp -r test/cli/autogen-pkg-autoloader "$tmp/test/cli"

cd "$tmp" || exit 1

mkdir output

"$cwd/main/sorbet" --silence-dev-message --stop-after=namer \
  --stripe-packages \
  -p autogen-autoloader:output \
  --autogen-autoloader-modules=RootPackage \
  --autogen-autoloader-exclude-require=byebug \
  --autogen-autoloader-ignore=scripts/ \
  --autogen-autoloader-preamble "$preamble" \
  --autogen-autoloader-pbal-namespaces RootPackage \
  test/cli/autogen-pkg-autoloader/{foo,bar,bar2,errors,__package}.rb \
  test/cli/autogen-pkg-autoloader/nested/*.rb \
  test/cli/autogen-pkg-autoloader/scripts/baz.rb 2>&1

for file in $(find output -type f | sort | grep -v "_mtime_stamp"); do
  printf "\n--- %s\n" "$file"
  cat "$file"
done

rm -rf "$tmp"
