#!/usr/bin/env bash

cwd="$(pwd)"
tmp="$(mktemp -d)"

mkdir -p "$tmp"/{alias,consumer,target}
cp test/cli/package-alias-import-autocorrect/alias/*.rb "$tmp/alias"
cp test/cli/package-alias-import-autocorrect/consumer/*.rb "$tmp/consumer"
cp test/cli/package-alias-import-autocorrect/target/*.rb "$tmp/target"

cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message --sorbet-packages --max-threads=0 -a . 2>&1
cat consumer/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" --silence-dev-message --sorbet-packages --max-threads=0 -a . 2>&1
cat consumer/__package.rb
