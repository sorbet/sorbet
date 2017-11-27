#!/bin/bash
set -e

cd "$(dirname $0)/../.."

targets="$(bazel query 'kind("cc_(library|binary|test)", //...)')"

cleanup() {
    rm -rf "tools/BUILD.tmp"
}
trap cleanup exit


(
    sed -n '0,/BEGIN compile_commands/p' tools/BUILD
    echo "$targets" | sed -e 's/^/"/' -e 's/$/",/'
    sed -n '/END compile_commands/,$p' tools/BUILD
) | buildifier > tools/BUILD.tmp

if [ "$1" == "-t" ]; then
    if ! diff -u tools/BUILD tools/BUILD.tmp; then
        echo "tools/BUILD needs to be updated." >&2
        echo "Please re-run:" >&2
        echo -e "  \e[97;1;42mtools/scripts/generate_compdb_targets.sh\e[0m" >&2
        echo "And commit the result." >&2
        exit 1
    fi
else
    mv -f tools/BUILD.tmp tools/BUILD
fi
