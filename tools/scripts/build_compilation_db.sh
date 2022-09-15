#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

if [ "$(uname -s)-$(uname -m)" = "Darwin-arm64" ]; then
  >&2 echo "Note: building the full compile_commands.json file is not yet supported on Apple Silicon machines."
  >&2 echo "For rudimentary support, comment out all //compiler: targets in tools/BUILD and re-run this script."
  >&2 echo
fi

./bazel build //tools:compdb --config=dbg

if command -v jq &> /dev/null; then
  jq . < bazel-bin/tools/compile_commands.json > compile_commands.json
else
  cp bazel-bin/tools/compile_commands.json compile_commands.json
fi
