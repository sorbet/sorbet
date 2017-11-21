#!/bin/bash

set -e

cd $(dirname $0)/../..

bazel build //tools:clang-tidy
./tools/scripts/build_compilation_db.sh

if [ "$#" -gt 0 ]; then
    cxx_src=("$@")
else
    cxx_src=(
        $(find . -path ./third_party -prune -false -o -name ".?*" -prune -false -o -name '*.cxx' -o -name '*.cc' )
    )
fi

# Explicitly pass our configuration. Somehow clang-tidy gets confused
# and doesn't respect it even though `-list-checks` and
# `-explain-config` appear to.
exec bazel-bin/tools/clang-tidy -config "$(cat .clang-tidy)" -quiet "${cxx_src[@]}"
