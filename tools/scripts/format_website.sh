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

yarn

if [ "$TEST" != "" ]; then
  if ! yarn prettier-check; then
    echo
    echo "^ To fix these files, run 'yarn prettier' in the website/ folder locally."
    exit 1
  fi
else
  yarn prettier
fi

