#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

bazel build //tools:compdb --config=dbg
sed "s,__EXEC_ROOT__,$(pwd)/bazel-sorbet_llvm," bazel-bin/tools/compile_commands.json > compile_commands.json
