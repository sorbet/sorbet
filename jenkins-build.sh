#!/bin/bash
set -eux
. /usr/stripe/bin/docker/stripe-init-build

cd "$( dirname "${BASH_SOURCE[0]}" )"

export BAZEL_BIN_LOC=/cache/bazel_binary

./tools/scripts/format_build_files.sh -t

cp bazelrc-jenkins .bazelrc

LSAN_OPTIONS=verbosity=1:log_threads=1 bazel test //... --test_output=errors
