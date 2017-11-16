#!/bin/bash
set -eux
. /usr/stripe/bin/docker/stripe-init-build

cd "$( dirname "${BASH_SOURCE[0]}" )"

export BAZEL_BIN_LOC=/cache/bazel_binary

cp bazelrc-jenkins .bazelrc

FILE=./ci/$JOB_NAME.sh
if [ ! -f $FILE ]; then
    echo "Unkonwn Job: $JOB_NAME. Should have $FILE"
fi
$FILE
