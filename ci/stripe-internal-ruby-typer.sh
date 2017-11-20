#!/bin/bash
set -eux

./tools/scripts/format_build_files.sh -t
./tools/scripts/format_cxx.sh -t
./tools/scripts/generate_compdb_targets.sh -t

err=0

# Disable leak sanatizer. Does not work in docker
# https://github.com/google/sanitizers/issues/764
bazel test //... --test_output=errors --test_env="ASAN_OPTIONS=detect_leaks=0" || err=$?

mkdir -p /log/junit
find bazel-testlogs/ -name '*.xml' | while read -r line; do
    cp "$line" /log/junit/"$(echo "${line#bazel-testlogs/}" | sed s,/,_,g)"
done

exit "$err"
