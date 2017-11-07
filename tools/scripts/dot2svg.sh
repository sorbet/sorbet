#!/bin/bash
set -e

rb_src=(
    $(find ./test/testdata/ -name '*.rb.cfg.exp')
)

for src in "${rb_src[@]}"; do
    out="$src.svg"
    dot -Tsvg "$src"  > "$out"
done
