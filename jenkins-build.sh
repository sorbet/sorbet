#!/bin/bash
set -eux
# shellcheck disable=SC1091
. /usr/stripe/bin/docker/stripe-init-build

cd "$( dirname "${BASH_SOURCE[0]}" )"

BAZEL_BIN_LOC="/cache/$(lsb_release -sr)/bazel_binary"
export BAZEL_BIN_LOC

cp bazelrc-jenkins .bazelrc

FILE=./ci/$JOB_NAME.sh
if [ ! -f "$FILE" ]; then
    echo "Unkonwn Job: $JOB_NAME. Should have $FILE"
fi
$FILE
