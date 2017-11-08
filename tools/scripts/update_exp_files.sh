#!/bin/bash
set -e

passes=(cfg parse-tree ast ast-raw name-table name-tree name-tree-raw)

bazel build //main:ruby-typer

rb_src=(
    $(find ./test/testdata/ -name '*.rb')
)

for src in "${rb_src[@]}"; do
    for pass in "${passes[@]}" ; do
        candidate="$src.$pass.exp"
        if [ -e "$candidate" ]; then
            echo "bazel-bin/main/ruby-typer --no-stdlib --print $pass $src > $candidate"
            bazel-bin/main/ruby-typer --no-stdlib --print "$pass" "$src" > "$candidate"
        fi
    done
done

./tools/scripts/dot2svg.sh
