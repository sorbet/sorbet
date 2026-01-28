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

./bazel build --config=dbg //tools:clang-format &> /dev/null

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

    if [ "${EMIT_SYNCBACK:-}" != "" ]; then
        echo '### BEGIN SYNCBACK ###'
        for file in "${misformatted[@]}"; do
            echo "$file"
        done
        echo '### END SYNCBACK ###'
    fi
else
    echo "Some c++ files need to be formatted! Please run:"
    echo ""
    echo '```sh'
    if [ "${#misformatted[@]}" -eq 1 ]; then
        # Single file - put everything on one line
        echo "./tools/scripts/format_cxx.sh ${misformatted[0]}"
    else
        # Multiple files - use multi-line format
        echo "./tools/scripts/format_cxx.sh \\"
        for i in "${!misformatted[@]}"; do
            if [ "$i" -eq $((${#misformatted[@]} - 1)) ]; then
                echo "  ${misformatted[$i]}"
            else
                echo "  ${misformatted[$i]} \\"
            fi
        done
    fi
    echo '```'
fi


exit 1
