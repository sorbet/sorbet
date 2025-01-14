#!/bin/bash

set -euo pipefail

sorbet="$(pwd)/main/sorbet"

tmp="$(mktemp -d)"
trap 'rm -rf $tmp' EXIT

cd "$tmp"
mkdir -p input references/lib
touch input/foo.rb references/bar.rb references/lib/baz.rb

cd input
ln -s ../references references
ln -s ../references/bar.rb bar.rb

echo '------------------------------------------------------------------------'
"$sorbet" --silence-dev-message -p file-table-json --no-stdlib --dir .

echo '------------------------------------------------------------------------'
"$sorbet" --silence-dev-message -p file-table-json --no-stdlib --dir . --dir ./references

echo '------------------------------------------------------------------------'
"$sorbet" --silence-dev-message -p file-table-json --no-stdlib --dir . --dir ./references/lib
