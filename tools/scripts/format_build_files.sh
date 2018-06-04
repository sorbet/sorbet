#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

buildifier=$(mktemp -t buildifier.XXXXXX)
cleanup() {
    rm "$buildifier"
}
trap cleanup EXIT

list_build_files() {
  git ls-files -c -m -o --exclude-standard -- '**/BUILD' | sed "s,^,$(pwd)/,g"
}

bazel run --script_path "$buildifier" @com_github_bazelbuild_buildtools//buildifier

if [ "$1" == "-t" ]; then
  OUTPUT=$(list_build_files | xargs "$buildifier" -v -mode=diff || :)
  if [ -n "$OUTPUT" ]; then
    echo -ne "\\e[1;31m"
    echo "☢️☢️  Some bazel files need to be reformatted! ☢️☢️"
    echo "$OUTPUT"
    echo -ne "\\e[0m"
    echo -e "✨✨ Run \\e[97;1;42m ./tools/scripts/format_build_files.sh\\e[0m to fix them up.  ✨✨"
    exit 1
  fi
else
  list_build_files | xargs "$buildifier" -v -mode=fix
fi
