#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

bazel build //tools:compdb
sed "s,__EXEC_ROOT__,$(bazel info execution_root)," bazel-bin/tools/compile_commands.json > compile_commands.json
