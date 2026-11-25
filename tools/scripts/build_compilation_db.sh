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
      >&2 echo 'Each <build_arg> is given to the Bazel command'
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

# Absolutely qualify `external/` paths into the output_base, so that running
# another target (like `format_cxx.sh`) doesn't make clangd fail to find all
# the files it's looking for.
#
# This is a bit bespoke, because those paths can show up either like
#
#     -Iexternal/
#     -Bexternal/
#     ... external/ ...
#     "external/..."
#
# and the `external/` substring could show up in other places (e.g.,
# `foo/external/bar`). But this regex seems to work on all paths we produce today.
output_base="$(./bazel info "${bazel_args[@]}" output_base 2> /dev/null)"
sed -E "s#([ \"]|-[BI])external/#\\1$output_base/external/#g" bazel-bin/tools/compile_commands.json > compile_commands.json

if command -v jq &> /dev/null; then
  jq . < compile_commands.json > compile_commands.json.tmp
  mv compile_commands.json.tmp compile_commands.json
fi

echo '[ .. ] Wrote ./compile_commands.json'
