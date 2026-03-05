#!/usr/bin/env bash

root="$(pwd)"

cd "$(dirname "${BASH_SOURCE[0]}")"

dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT

sources=(a b c d e)

cp -r "${sources[@]}" "$dir"

"$root/main/sorbet" \
  --censor-for-snapshot-tests --silence-dev-message --max-threads=0 \
  --sorbet-packages -a "$dir" 2>&1

"$root/main/sorbet" \
  --censor-for-snapshot-tests --silence-dev-message --max-threads=0 \
  --sorbet-packages "$dir" 2>&1
