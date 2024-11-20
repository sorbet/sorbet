#!/bin/bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/../.."
# we are now at the repo root.

./bazel build \
  //test/cli:update //test/lsp:update \
  -c opt "$@"

./bazel test \
  //gems/sorbet-runtime:update_call_validation \
  //test/cli:update //test/lsp:update -c opt "$@"

tools/scripts/update_testdata_exp.sh
