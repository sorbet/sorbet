#!/bin/bash

set -e

cd "$(dirname "$0")"
cd ../..
# we are now at the repo root.

if ! command -v parallel &> /dev/null; then
    echo "This script requires GNU parallel to be installed"
    exit 1
fi

COMMAND_FILE="$(mktemp)"
trap 'rm -f "$COMMAND_FILE"' EXIT

bazel build //main:sorbet

if [ $# -eq 0 ]; then
    paths=(test/testdata)
else
    paths=("$@")
fi

rb_src=()
while IFS='' read -r line; do
    rb_src+=("$line")
done < <(find "${paths[@]}" -name '*.rb*' | sort)

basename=
srcs=()
exp_extensions="llo ll stderr"

for this_src in "${rb_src[@]}" DUMMY; do
    this_base="${this_src%__*}"
    if [ "$this_base" = "$basename" ]; then
        srcs=("${srcs[@]}" "$this_src")
        continue
    fi

    has_exp=
    for ext in $exp_extensions; do
        if [ -f "${this_base%.rb}.$ext.exp" ]; then
            has_exp=1
        fi
    done

    if [ -z $has_exp ]; then
        continue
    fi

    if [ -n "$basename" ]; then
        llvmir=$(mktemp -d)
        # shellcheck disable=SC1010
        echo f\(\) \{ \
            bazel-bin/main/sorbet \
            --silence-dev-message \
            --no-error-count \
            --llvm-ir-folder \
            "$llvmir" \
            "${srcs[@]}" \
            2\> "$llvmir/update_testdata_exp.stderr"\; \
            for ext in "$exp_extensions"\; do \
                exp=${basename%.rb}.\$ext.exp\; \
                if [ -f \$exp ]\; then \
                    cat "$llvmir/*.\$ext" \> \$exp\; \
                    fi\; \
                done\; \
                >> "$COMMAND_FILE" \
                \}\; f >> "$COMMAND_FILE"
    fi

    basename="$this_base"
    srcs=("$this_src")
done

parallel --joblog - < "$COMMAND_FILE"
