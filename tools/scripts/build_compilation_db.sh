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
bazel_args=(
  "--ui_event_filters=-info,-stdout,-stderr"
  "--config=dbg"
)
if ! ./bazel build "${bazel_args[@]}" //tools:compdb "$@" >&2; then
  if ! [ -t 1 ]; then
    echo '[ERR!] Failed to build //tools:compdb'
    echo 'Check the logs, or run this locally to reproduce:'
    echo
    echo '    ./bazel build --config=dbg //tools:compdb'
  fi
  exit 1
fi

if command -v jq &> /dev/null; then
  jq . < bazel-bin/tools/compile_commands.json > compile_commands.json
else
  cp bazel-bin/tools/compile_commands.json compile_commands.json
fi

echo '[ .. ] Wrote ./compile_commands.json'
