#!/usr/bin/env bash

cwd="$(pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

cp -RL test/cli/package-gen-packages-alias-prefixes/{producer,consumer} "$tmp/"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message --sorbet-packages --max-threads=0 \
  --censor-for-snapshot-tests --gen-packages -a . 2>&1
cat producer/__package.rb
