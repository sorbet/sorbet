#!/bin/bash
set -e

dot=$(mktemp)
svg=$(mktemp)
cleanup() {
    rm -r "$dot" "$svg"
}
trap cleanup EXIT

src=$1
if [ ! -f "$src" ]; then
    echo "$src does not exist"
    exit
fi

dir="$( dirname "${BASH_SOURCE[0]}" )"
"$dir"/../../bazel-bin/main/ruby-typer --suppress-non-critical --print cfg "$src" > "$dot"
dot -Tsvg "$dot" > "$svg"
open -a "Google Chrome" "$svg"
