#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

#   NOTE on `--config=dbg` and `--remote_download_outputs=all`
#
# We assume that you'll be iterating locally with `--config=dbg` builds.
# If you choose to use something else, be sure to:
#
# - edit this script and then (re-)generat the compile_commands.json file
# - pass `--remote_download_outputs=all` when building
#
# (The `--config=dbg` configuration already passes that flag.) Without this
# flag, intermediate generated outputs will not be placed in the `bazel-bin/`
# folder if they have been cached, which means that clangd won't find them.
#
# TODO(jez) Do other compile commands generators work better here?
./bazel build //tools:compdb --config=dbg "$@"

if command -v jq &> /dev/null; then
  jq . < bazel-bin/tools/compile_commands.json > compile_commands.json
else
  cp bazel-bin/tools/compile_commands.json compile_commands.json
fi
