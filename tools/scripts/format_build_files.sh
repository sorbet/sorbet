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
    echo "Some bazel files need to be formatted!"
    echo "\`\`\`"
    bazel run "${bazel_args[@]}" //test/lint/buildifier:diff 2> /dev/null || true
    echo "\`\`\`"
    echo -e "Run \`./tools/scripts/format_build_files.sh\` to format them."
    exit 1
  fi
else
  bazel run //test/lint/buildifier:fix
fi
