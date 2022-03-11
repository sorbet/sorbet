#!/usr/bin/env bash

set -euo pipefail

test_path="$(dirname "$0")"

sorbet="$PWD/main/sorbet"

rbis="$(mktemp -d)"
trap "rm -rf $rbis" EXIT

echo "-- sanity-checking package with --stripe-packages"
"$sorbet" --silence-dev-message \
  --stripe-packages \
  --dump-package-info="$rbis/package-info.json" \
  --extra-package-files-directory-prefix="${test_path}/other/" \
  "$test_path"

show_output() {
  local name=$1
  local label=$2
  local suffix=$3
  local file="${rbis}/${name}_Package.${suffix}"
  if [ -f "$file" ]; then
    echo "-- ${label} (${name})"
    cat "${file}"
  fi
}

# Generate rbis for each package
find . -name __package.rb | sort | while read -r package; do
  name="$(awk '/class/ {print $2}' "$package")"

  echo "-- $package ($name)"

  "$sorbet" --silence-dev-message \
    --ignore=__package.rb \
    --package-rbi-generation \
    --package-rbi-dir="$rbis" \
    --extra-package-files-directory-prefix="${test_path}/other/" \
    --single-package="$name" "$test_path"

  show_output "$name" "RBI"                   "package.rbi"
  show_output "$name" "RBI Deps"              "deps.json"
  show_output "$name" "Test Private RBI"      "test.private.package.rbi"
  show_output "$name" "Test Private RBI Deps" "test.private.deps.json"
  show_output "$name" "Test RBI"              "test.package.rbi"
  show_output "$name" "Test RBI Deps"         "test.deps.json"
done

