#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

bazel build //tools:compdb
sed "s,__EXEC_ROOT__,$(pwd)/bazel-ruby-typer," bazel-bin/tools/compile_commands.json > compile_commands.json
