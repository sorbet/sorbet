#!/bin/bash
set -e

passes=(cfg parse-tree ast ast-raw name-table name-tree name-tree-raw)

bazel build //main:ruby-typer --config=unsafe -c opt

rb_src=("$@")

if [ -z "${rb_src[*]}" ]; then
    rb_src=(
        $(find ./test/testdata/ -name '*.rb' | sort)
    )
fi

basename=
srcs=()

for this_src in "${rb_src[@]}" DUMMY; do
    this_base=${this_src%__*}
    if [ "$this_base" = "$basename" ]; then
        srcs=("${srcs[@]}" "$this_src")
        continue
    fi

    if [ "$basename" ]; then
        for pass in "${passes[@]}" ; do
            candidate="$basename.$pass.exp"
            if [ -e "$candidate" ]; then
                echo "bazel-bin/main/ruby-typer --print $pass ${srcs[@]} > $candidate"
                bazel-bin/main/ruby-typer --print "$pass" "${srcs[@]}" > "$candidate" 2>/dev/null
            fi
        done
    fi

    basename="$this_base"
    srcs=("$this_src")
done

./tools/scripts/dot2svg.sh

bazel-bin/main/ruby-typer test/end-to-end-test-input.rb  2>&1 | sed -e 's,\(rbi/stdlib.rbi:\)[0-9]*,\1__LINE__,' > test/end-to-end-test-output
