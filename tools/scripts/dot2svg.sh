#!/bin/bash
set -e

# shellcheck disable=SC2207
rb_src=(
    $(find ./test/testdata -name '*.rb.cfg.exp' -or -name '*.rb.cfg-raw.exp')
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
        continue
    fi
    (
        dot -Tsvg "$src"
        echo "<!-- ruby-types:sha256: $cur_sha -->"
    ) > "$out"
done
