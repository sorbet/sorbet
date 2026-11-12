#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

#   NOTE on `--config=dbg` and `--remote_download_outputs=all`
#
# We assume that you'll be iterating locally with `--config=dbg` builds, so
# the compile commands should use the same flags. If you build with some other
# config, pass it here (for example `--config=dev`), and be sure that it passes
# `--remote_download_outputs=all`, like `--config=dbg` and `--config=dev` do.
# Without this flag, intermediate generated outputs will not be placed in the
# `bazel-bin/` folder if they have been cached, which means that clangd won't
# find them.
bazel_args=(
)
for arg in "$@"; do
  case "$arg" in
    -h|--help)
      >&2 echo "usage: $0 [<build_arg> ...]"
      >&2 echo
      >&2 echo 'Each <build_arg> is given to the `bazel build` command'
      exit 0
      ;;
  esac
  bazel_args+=("$arg")
done

if [ "${#bazel_args[@]}" = 0 ]; then
  bazel_args+=("--config=dbg")
fi

ui_event_filter="--ui_event_filters=-info,-stdout,-stderr"
if ! ./bazel build "$ui_event_filter" "${bazel_args[@]}" //tools:compdb >&2; then
  if ! [ -t 1 ]; then
    echo '[ERR!] Failed to build //tools:compdb'
    echo 'Check the logs, or run this locally to reproduce:'
    echo
    echo "    ./bazel build ${bazel_args[*]} //tools:compdb"
  fi
  exit 1
fi

if command -v jq &> /dev/null; then
  jq . < bazel-bin/tools/compile_commands.json > compile_commands.json
else
  cp bazel-bin/tools/compile_commands.json compile_commands.json
fi

echo '[ .. ] Wrote ./compile_commands.json'
