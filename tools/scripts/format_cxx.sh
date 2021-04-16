#!/bin/bash

set -e

mode="fix"

while getopts 't' opt; do
    case "$opt" in
        t)
            mode="test"
            ;;
        *)
            break
            ;;
    esac
done

shift $((OPTIND - 1))

cd "$(dirname "$0")/../.."

./bazel build @com_stripe_ruby_typer//tools:clang-format &> /dev/null

if [ "$#" -ne 0 ]; then
    cxx_src=("$@")
else
    # shellcheck disable=SC2207
    cxx_src=(
        $(git ls-files -c -m -o --exclude-standard -- '*.cxx' '*.cc' '*.h' '*.c' | \
              grep -v ^third_party/
        )
    )
fi

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
    bazel-bin/external/com_stripe_ruby_typer/tools/clang-format -style=file "$src" > "$src.formatted"
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
else
    echo -ne "\\e[1;31m" >&2
    echo "The following files are misformatted!" >&2
    echo -ne "\\e[0m" >&2
    echo -e "Run \\e[97;1;42m ./tools/scripts/format_cxx.sh \\e[0m to format." >&2
fi

for src in "${misformatted[@]}"; do
    echo "$src" >&2
done

if [ "${EMIT_SYNCBACK:-}" != "" ] && [ "${#misformatted[@]}" -ne 0 ]; then
    echo '### BEGIN SYNCBACK ###'
    for file in "${misformatted[@]}"; do
        echo "$file"
    done
    echo '### END SYNCBACK ###'
fi

exit 1
