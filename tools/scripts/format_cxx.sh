#!/bin/bash

set -e

if [ "$1" = -t ]; then
    mode="test"
else
    mode="fix"
fi

cd $(dirname $0)/../..

cxx_src=(
    $(find . -name '*.cxx' -o -name '*.h' -o -name '*.cc')
)
misformatted=()

cleanup() {
    for src in "${cxx_src[@]}"; do
        rm -f "$src.formatted"
    done
}

trap cleanup EXIT

## uncomment the line below to dump format
#clang-format -style=file -dump-config

for src in "${cxx_src[@]}"; do
    clang-format -style=file "$src" > "$src.formatted"
    if ! cmp -s "$src" "$src.formatted"; then
        misformatted=("${misformatted[@]}" "$src")
        if [ "$mode" = "fix" ]; then
            cp "$src.formatted" "$src"
        fi
    fi

    rm -f "$src.formatted"
done

if [ -z "${misformatted[*]}" ]; then
    exit 0
fi

if [ "$mode" = "fix" ]; then
    echo "Formatted the following files:" >&2
else
    echo -ne "\e[1;31m"
    echo "The following files are misformatted!" >&2
    echo -ne "\e[0m"
    echo -e "Run \e[97;1;42m ./tools/scripts/format_cxx.sh \e[0m to format." >&2
fi

for src in "${misformatted[@]}"; do
    echo "$src"
done

exit 1
