#!/bin/bash

set -exuo pipefail

bazel build //test/fuzz:fuzz_dash_e --config=fuzz -c opt

# shellcheck disable=SC2207
input_src=(
    $(find 'fuzz_crashers/original' -name "crash-*" | sort)
)

COMMAND_FILE=$(mktemp)

for this_src in "${input_src[@]}" DUMMY; do
    if [[ ! "$this_src" == DUMMY ]]; then
        echo "./tools/scripts/fuzz_minimize_crash.sh \"$this_src\" 2>/dev/null" >> "$COMMAND_FILE"
    fi
done

parallel --joblog - < "$COMMAND_FILE"
