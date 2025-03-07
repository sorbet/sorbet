#!/bin/bash

set -euo pipefail

if [ "$#" -eq 0 ]; then
  >&2 echo "usage: $0 src.rb [...] > src.rb.autocorrects.rb"
  exit 1
fi

if command -v gcp &> /dev/null; then
  cp="gcp" # GNU cp
elif [ "$(uname)" = "Linux" ]; then
  cp="cp"
else
  if cp --parents 2>&1 | grep -q 'illegal option'; then
    echo "This script requires the --parents flag provided by GNU cp"
    echo "Install GNU coreutils for your platform or re-run this script on Linux"
    exit 1
  fi
fi

args=("${@}")
srcs=()
options=(
  --silence-dev-message
  --suppress-non-critical
  --censor-for-snapshot-tests
  --autocorrect
  "--max-threads=0"
)

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

for arg in "${args[@]}"; do
  case "$arg" in
    -*)
      options+=("$arg")
      ;;
    *)
      src="$arg"
      srcs+=("$src")
      "$cp" --parents "$src" "$tmpdir"
      ;;
  esac
done

if grep -q '^# enable-suggest-unsafe: true$' "${srcs[@]}"; then
  options+=("--suggest-unsafe")
fi

cwd="$PWD"
pushd "$tmpdir" &> /dev/null

"$cwd"/bazel-bin/main/sorbet \
  "${options[@]}" \
  "${srcs[@]}" \
  &> /dev/null || true

for src in "${srcs[@]}"; do
  echo "# -- $src --"
  cat "$src"
  echo '# ------------------------------'
done

popd &> /dev/null
