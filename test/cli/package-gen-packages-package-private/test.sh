#!/bin/bash

cwd="$(pwd)"

sorbet_args=(--did-you-mean=false --max-threads=0 --censor-for-snapshot-tests --silence-dev-message --sorbet-packages)

tmp="$(mktemp -d)"
cd test/cli/package-gen-packages-package-private || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

echo "###### Running gen-packages (first run) ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" --gen-packages=strict -a . 2>&1

echo
echo "###### cat private_consumer/__package.rb ######"
cat private_consumer/__package.rb

echo
echo "###### Running gen-packages (second run) ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" --gen-packages=strict -a . 2>&1

echo
echo "###### cat private_consumer/__package.rb ######"
cat private_consumer/__package.rb
