#!/bin/bash

set -euo pipefail

usage() {
  cat <<EOF

tools/scripts/format_website.sh: Format JS, CSS, and Markdown files in website/

Usage:
  tools/scripts/format_website.sh [options]

Options:
  -t, --test   Dry-run. Only test whether things need to be formatted.
               [default: Formats in place]
EOF
}

TEST=
while [[ $# -gt 0 ]]; do
  case $1 in
    -t|--test)
      TEST="--test"
      shift
      ;;
    -*)
      echo "Unrecognized option: $1" >&2 && usage && exit 1
      ;;
    *)
      echo "Unrecognized positional argument: $1" >&2 && usage && exit 1
      ;;
  esac
done

cd "$(dirname "$0")/../../website"

yarn --silent

if [ "$TEST" != "" ]; then
  trap "rm prettier.output" exit
  if ! yarn --silent prettier-lint > prettier.output; then
    echo "Some docs need to be formatted!"
    sed 's/^/* /' prettier.output
    echo
    echo "Run \`./tools/scripts/format_website.sh\` to format them"
    exit 1
  fi
else
  yarn prettier
fi

