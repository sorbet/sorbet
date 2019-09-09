#!/bin/bash
set -e

dot=$(mktemp)
svg=$(mktemp).svg

dir="$( dirname "${BASH_SOURCE[0]}" )"
if [ "${RAW:-}" != "" ]; then
  print="cfg-raw"
else
  print="cfg"
fi
"$dir"/../../bazel-bin/main/sorbet --silence-dev-message --suppress-non-critical --print "$print" "$@" > "$dot"
dot -Tsvg "$dot" > "$svg"
if command -v open &> /dev/null; then
  open -a "Google Chrome" "$svg"
elif command -v sensible-browser &> /dev/null; then
  sensible-browser "$svg"
elif command -v xdg-open &> /dev/null; then
  xdg-open "$svg"
else
  echo "$svg"
fi
