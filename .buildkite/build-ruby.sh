#!/bin/bash

set -euo pipefail

echo "--- Pre-setup"

export JOB_NAME=build-ruby
source .buildkite/tools/setup-bazel.sh

PATH=$PATH:$(pwd)
export PATH

bazel test @ruby_2_4_3//:smoke_test
