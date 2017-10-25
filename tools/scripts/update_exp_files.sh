#!/bin/bash
set -e

passes=(cfg parse-tree ast name-table name-tree)

bazel build //main:ruby-typer

rb_src=(
    $(find ./test/testdata/ -name '*.rb')
)

for src in "${rb_src[@]}"; do
    for pass in "${passes[@]}" ; do
        candidate="$src.$pass.exp"
        if [ -e "$candidate" ]; then
            echo "bazel-bin/main/ruby-typer --print $pass $src > $candidate"
            bazel-bin/main/ruby-typer --print "$pass" "$src" > "$candidate"
        fi
    done
done
