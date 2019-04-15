#!/bin/bash
set -e

COMMAND_FILE=$(mktemp)

echo 'RECORD=1 ruby gems/sorbet/test/srb-rbi/empty/empty.rb && patch -p1 < gems/sorbet/test/srb-rbi/empty/0001-Revert-laptop-patch.patch' >> "$COMMAND_FILE"

passes=(parse-tree parse-tree-json ast ast-raw dsl-tree dsl-tree-raw symbol-table symbol-table-raw name-tree name-tree-raw resolve-tree resolve-tree-raw cfg autogen)

bazel build test/cli:update test/lsp:update //main:sorbet-light -c opt "$@"

# shellcheck disable=SC2207
rb_src=(
    $(find test/testdata -name '*.rb' | sort)
)

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
            args=()
            if [ "$pass" = "autogen" ]; then
                args=("--stop-after=namer --skip-dsl-passes")
            fi
            if [ -e "$candidate" ]; then
                echo bazel-bin/main/sorbet-light --silence-dev-message  --suppress-non-critical --print "$pass" --max-threads 0 "${args[@]}" "${srcs[@]}" \> "$candidate" 2\>/dev/null >> "$COMMAND_FILE"
            fi
        done
    fi

    basename="$this_base"
    srcs=("$this_src")
done

parallel --joblog - < "$COMMAND_FILE"

bazel test test/cli:update test/lsp:update -c opt "$@"
