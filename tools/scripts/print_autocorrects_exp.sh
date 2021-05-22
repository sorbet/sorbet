#!/bin/bash

set -euo pipefail

if [ "$#" -eq 0 ]; then
  >&2 echo "usage: $0 src.rb [...] > src.rb.autocorrects.rb"
  exit 1
fi

srcs=("${@}")

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

srcs_basenames=()
for src in "${srcs[@]}"; do
  src_basename="$(basename "$src")"
  if [ -f "$tmpdir/$src_basename" ]; then
    >&2 echo "error: duplicate file basenames: $src_basename"
    exit 1
  fi

  srcs_basenames+=("$src_basename")
  cp "$src" "$tmpdir"
done

options=(
  --silence-dev-message
  --suppress-non-critical
  --censor-for-snapshot-tests
  --autocorrect
  "--max-threads=0"
)

if grep -q '^# enable-suggest-unsafe: true$' "${srcs[@]}"; then
  options+=("--suggest-unsafe")
fi

cwd="$PWD"
pushd "$tmpdir" &> /dev/null

"$cwd"/bazel-bin/main/sorbet \
  "${options[@]}" \
  "${srcs_basenames[@]}" \
  &> /dev/null || true

i=-1
for src in "${srcs[@]}"; do
  i=$((i + 1))
  src_basename="$tmpdir/${srcs_basenames[$i]}"
  echo "# -- $src --"
  cat "$src_basename"
  echo '# ------------------------------'
done

popd &> /dev/null
