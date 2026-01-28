#!/bin/bash

set -e

cd "$(dirname "$0")/../.."


if [ "$1" == "-t" ]; then

  # quieter bazel output
  bazel_args=(
    "--ui_event_filters=-info,-stdout,-stderr"
    "--noshow_progress"
  )
  if ! bazel run "${bazel_args[@]}" //test/lint/buildifier:lint &> /dev/null; then
    echo "Some bazel files need to be formatted! Please run:"
    echo
    echo '```sh'
    echo "./tools/scripts/format_build_files.sh"
    echo '```'
    echo
    echo "To set up your editor to format on save, run:"
    echo
    echo '```sh'
    echo "bazel build @com_github_bazelbuild_buildtools//buildifier:buildifier"
    echo '```'
    echo
    echo "then copy the resulting binary out of bazel-bin/ onto your PATH, and configure your editor to run this executable on save."
    echo
    echo "(The bazel-bin/ PATH is not stable, and might get blown away for various reasons, so copying it out ensures that it's always available.)"
    exit 1
  fi
else
  bazel run //test/lint/buildifier:fix
fi
