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

if [ "${REMOTE:-}" != "" ]; then
  # Finds the path you are currently in (relative to HOME) and assumes that a
  # Sorbet repo exists in an identically named path on the remote server
  # (relative to HOME on the remote server). File paths in the command line are
  # remote paths, relative to the remote server's sorbet repo.
  #
  # Example:
  #
  #   REMOTE='jez@my.host.com' tools/scripts/cfg-view.sh only_on_remote.rb
  #
  relpath="$(realpath --relative-to "$HOME" "$PWD")"
  sorbet_exe=(ssh "$REMOTE" -- cd "\$HOME/$relpath" \; bazel-bin/main/sorbet)
else
  sorbet_exe=("$dir"/../../bazel-bin/main/sorbet)
fi

"${sorbet_exe[@]}" --silence-dev-message --suppress-non-critical --typed=true --print "$print" "$@" > "$dot"
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
