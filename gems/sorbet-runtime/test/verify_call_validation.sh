#!/usr/bin/env bash

set -euo pipefail

mode="$1"
committed="$2"
generated="$3"

if [ "$mode" = "test" ]; then
  if ! diff -u "$committed" "$generated"; then
    echo
    echo "Generated and committed files did not match. Diff is above."
    exit 1
  fi
elif [ "$mode" = "update" ]; then
  cat "$generated" > "$committed"
else
  echo "Invalid mode: $mode"
  exit 1
fi
