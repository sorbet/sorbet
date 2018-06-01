#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

bazel build //tools:clang-tidy
./tools/scripts/build_compilation_db.sh

got_cc=
for arg in "$@"; do
    if [ "${arg%.cc}" != "$arg" ]; then
        got_cc=1
        break
    fi
done
if [ "$got_cc" ]; then
    cxx_src=()
else
    # shellcheck disable=SC2207
    cxx_src=(
        $(git ls-files -c -m -o --exclude-standard -- '*.cxx' '*.cc' | \
              grep -v ^third_party/
        )
    )
fi

# Explicitly pass our configuration. Somehow clang-tidy gets confused
# and doesn't respect it even though `-list-checks` and
# `-explain-config` appear to.
exec bazel-bin/tools/clang-tidy -config "$(cat .clang-tidy)" -quiet "$@" "${cxx_src[@]}"
