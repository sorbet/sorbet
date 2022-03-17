#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

./bazel build //tools:compdb --config=dbg

execution_root="$(./bazel info execution_root)"

sed "s|__EXEC_ROOT__|$execution_root|g" bazel-bin/tools/compile_commands.json > compile_commands.json
