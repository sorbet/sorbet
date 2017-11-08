#!/bin/bash
set -e

rb_src=(
    $(find ./test/testdata/ -name '*.rb.cfg.exp')
)

for src in "${rb_src[@]}"; do
    out="$src.svg"
    if test -f "$out"; then
        orig_sha=$(tail -n 1 "$out" | grep -Eo 'ruby-types:sha256: [a-f0-9]+' | cut -f2 -d' ')
    else
        orig_sha=""
    fi

    cur_sha=$(shasum -a 256 "$src" | cut -f1 -d ' ')
    if [ "$orig_sha" = "$cur_sha" ]; then
        echo "Not regenerating $out..."
        continue
    fi
    (
        dot -Tsvg "$src"
        echo "<!-- ruby-types:sha256: $cur_sha -->"
    ) > "$out"
done
