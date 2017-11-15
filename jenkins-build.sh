#!/bin/bash
set -eux
. /usr/stripe/bin/docker/stripe-init-build

cd "$( dirname "${BASH_SOURCE[0]}" )"

export BAZEL_BIN_LOC=/cache/bazel_binary

cp bazelrc-jenkins .bazelrc

./tools/scripts/format_build_files.sh -t
./tools/scripts/format_cxx.sh -t

# Disable leak sanatizer. Does not work in docker
# https://github.com/google/sanitizers/issues/764

err=0
bazel test //... --test_output=errors --test_env="ASAN_OPTIONS=detect_leaks=0" --test_env="LSAN_OPTIONS=verbosity=1:log_threads=1" || err=$?

mkdir -p /log/junit
cp -a bazel-testlogs/. /log/junit

exit "$err"
