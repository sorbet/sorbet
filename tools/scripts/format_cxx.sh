#!/bin/bash

set -e

if [ "$1" = -t ]; then
    mode="test"
else
    mode="fix"
fi

cd "$(dirname "$0")/../.."

./bazel build //tools:clang-format &> /dev/null

# shellcheck disable=SC2207
cxx_src=(
    $(git ls-files -c -m -o --exclude-standard -- '*.cxx' '*.cc' '*.h' | \
          grep -v ^third_party/
    )
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
    bazel-bin/tools/clang-format -style=file "$src" > "$src.formatted"
    if ! cmp -s "$src" "$src.formatted"; then
        misformatted=("${misformatted[@]}" "$src")
        if [ "$mode" = "fix" ]; then
            cp "$src.formatted" "$src"
        fi
    fi

    rm -f "$src.formatted"
done

if [ "${#misformatted[@]}" -eq 0 ]; then
    exit 0
fi

if [ "$mode" = "fix" ]; then
    echo "Formatted the following files:" >&2
    for src in "${misformatted[@]}"; do
        echo "$src" >&2
    done
else
    echo "Some c++ files need to be formatted!"
    for src in "${misformatted[@]}"; do
        echo "* $src"
    done
    echo ""
    echo "Run \`./tools/scripts/format_cxx.sh\` to format them"
fi


exit 1
