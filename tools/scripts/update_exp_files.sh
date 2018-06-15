#!/bin/bash
set -e

COMMAND_FILE=$(mktemp)

passes=(parse-tree parse-tree-json ast ast-raw dsl-tree dsl-tree-raw name-table name-tree name-tree-raw cfg cfg-raw typed-source)

bazel build //main:sorbet -c opt

rb_src=("$@")

if [ -z "${rb_src[*]}" ]; then
    # shellcheck disable=SC2207
    rb_src=(
        $(find test/testdata -name '*.rb' | sort)
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
                echo bazel-bin/main/sorbet  --suppress-non-critical --print "$pass" --max-threads 1 "${srcs[@]}" \> "$candidate" 2\>/dev/null >> "$COMMAND_FILE"
            fi
        done
    fi

    basename="$this_base"
    srcs=("$this_src")
done

parallel --joblog - < "$COMMAND_FILE"

bazel test -c opt test/cli:update
