#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

bazel build //tools:compdb -c opt

execution_root="$(bazel info execution_root)"

sed "s|__EXEC_ROOT__|$execution_root|" bazel-bin/tools/compile_commands.json > compile_commands.json
