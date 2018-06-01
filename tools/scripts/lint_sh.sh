#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

list_sh_src() {
    git ls-files -z -c -m -o --exclude-standard -- '*.sh'
}

if [ "$1" == "-t" ]; then
  OUTPUT=$(list_sh_src | xargs -0 shellcheck -s bash)
  if [ -n "$OUTPUT" ]; then
    echo -ne "\\e[1;31m" >&2
    echo "Some shell files have lint errors!" >&2
    echo "$OUTPUT" >&2
    echo -ne "\\e[0m" >&2
    echo -e "Run \\e[97;1;42m ./tools/scripts/lint_sh.sh\\e[0m to see the errors."
    exit 1
  fi
else
    list_sh_src | xargs -0 shellcheck -s bash
fi
