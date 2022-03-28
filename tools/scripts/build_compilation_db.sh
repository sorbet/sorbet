#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

./bazel build //tools:compdb --config=dbg

cp bazel-bin/tools/compile_commands.json compile_commands.json
