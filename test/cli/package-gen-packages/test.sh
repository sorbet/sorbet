#!/bin/bash

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-gen-packages || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1


"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --gen-packages --packager-layers=util,app -a . 2>&1

"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --lsp --sorbet-packages --gen-packages --packager-layers=util,app -a . 2>&1

"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --sorbet-packages --gen-packages --packager-layers=util,app  --gen-packages-update-visibility-for=B --gen-packages-update-visibility-for=E --gen-packages-allow-relaxing-test-visibility -a . 2>&1

cat B/__package.rb

cat C/__package.rb

cat D/__package.rb

cat E/__package.rb
