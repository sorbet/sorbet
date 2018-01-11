#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

if [ "$1" == "-t" ]; then
  OUTPUT=$(find . -name '*.sh' -exec shellcheck -s bash {} \;)
  if [ -n "$OUTPUT" ]; then
    echo -ne "\\e[1;31m" >&2
    echo "Some shell files have lint errors!" >&2
    echo "$OUTPUT" >&2
    echo -ne "\\e[0m" >&2
    echo -e "Run \\e[97;1;42m ./tools/scripts/lint_sh.sh\\e[0m to see the errors."
    exit 1
  fi
else
  exec find . -name '*.sh' -exec shellcheck -s bash {} \;
fi
