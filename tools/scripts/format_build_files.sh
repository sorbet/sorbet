#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

if [ "$1" == "-t" ]; then
  if ! bazel run @com_stripe_ruby_typer//test/lint/buildifier:lint &> /dev/null; then
    echo -ne "\\e[1;31m"
    echo "☢️☢️  Some bazel files need to be reformatted! ☢️☢️"
    echo "$OUTPUT"
    echo -ne "\\e[0m"
    echo -e "✨✨ Run \\e[97;1;42m ./tools/scripts/format_build_files.sh\\e[0m to fix them up.  ✨✨"
    bazel run @com_stripe_ruby_typer//test/lint/buildifier:diff
    exit 1
  fi
else
  bazel run @com_stripe_ruby_typer//test/lint/buildifier:fix
fi
