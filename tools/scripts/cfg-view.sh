#!/bin/bash
set -e

dot=$(mktemp)
svg=$(mktemp)
cleanup() {
    rm -r $dot $svg
}
trap cleanup EXIT

src=$1
if [ ! -f "$src" ]; then
    echo "$src does not exist"
    exit
fi

bazel-bin/main/ruby-typer --print cfg "$src" > "$dot"
dot -Tsvg "$dot" > "$svg"
open -a "Google Chrome" "$svg"
