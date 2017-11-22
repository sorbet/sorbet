#!/bin/bash
set -e

passes=(cfg parse-tree ast ast-raw name-table name-tree name-tree-raw)

bazel build //main:ruby-typer --config=unsafe -c opt

rb_src=("$@")

if [ -z "${rb_src[*]}" ]; then
    rb_src=(
        $(find ./test/testdata/ -name '*.rb')
    )
fi

for src in "${rb_src[@]}"; do
    for pass in "${passes[@]}" ; do
        candidate="$src.$pass.exp"
        if [ -e "$candidate" ]; then
            echo "bazel-bin/main/ruby-typer --print $pass --set-freshNameId=10000 $src > $candidate"
            bazel-bin/main/ruby-typer --print "$pass" --set-freshNameId=10000 "$src" > "$candidate"
        fi
    done
done

./tools/scripts/dot2svg.sh
