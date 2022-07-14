#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

./bazel build //tools:compdb --config=dbg

if command -v jq &> /dev/null; then
  jq . < bazel-bin/tools/compile_commands.json > compile_commands.json
else
  cp bazel-bin/tools/compile_commands.json compile_commands.json
fi
