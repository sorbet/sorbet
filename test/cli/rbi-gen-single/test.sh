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
  "$test_path"

# Generate rbis for each package
find . -name __package.rb | sort | while read -r package; do
  name="$(awk '/class/ {print $2}' "$package")"

  echo "-- $package ($name)"

  "$sorbet" --silence-dev-message \
    --ignore=__package.rb,"$rbis" \
    --package-rbi-output="$rbis" \
    --single-package="$name" "$test_path"

  echo "-- RBI: $package ($name)"
  cat "$rbis/${name//::/_}_Package.package.rbi"

  echo "-- JSON: $package ($name)"
  cat "$rbis/${name//::/_}_Package.deps.json"

done

