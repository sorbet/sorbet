#!/bin/bash
set -eux
. /usr/stripe/bin/docker/stripe-init-build

cd "$( dirname "${BASH_SOURCE[0]}" )"

export BAZEL_BIN_LOC=/cache/bazel_binary

cp bazelrc-jenkins .bazelrc

if [[ "$JOB_NAME" == stripe-internal-ruby-typer-pay-server ]]; then
    ./ci/check_pay_server.sh
elif [[ "$JOB_NAME" == stripe-internal-ruby-typer ]]; then
    ./ci/run_tests.sh
else
    echo "Unkonwn Job: $JOB_NAME"
fi
