#!/bin/bash

set -e

cd "$(dirname "$0")"
base_dir="$(pwd)"
cd ../../../..
# we are now at the repo root

versions=("sorbet_ruby_2_7" "sorbet_ruby_3_0")

if ! bazel test //gems/sorbet/test/hidden-method-finder -c opt "$@"; then
    for test_dir in bazel-bin/gems/sorbet/test/hidden-method-finder/{simple,thorough}; do
        echo "Updating $test_dir"
        suite="$(basename "${test_dir}")"
        for version in "${versions[@]}"; do
            updated="${test_dir}/actual_${version}.rbi"
            previous="${base_dir}/${suite}/${version}_hidden.rbi.exp"
            if ! diff "$updated" "$previous" >/dev/null ; then
                echo "$suite with $version differs; updating"
                cp "$updated" "$previous"
            fi
        done
    done
else
    echo "Hidden methods tests are up-to-date"
fi
