#!/bin/bash

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-gen-packages-strict || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1


echo "###### gen-packages=strict mode not allowed with gen-packages-update-visibility-for ######"

"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --sorbet-packages --gen-packages=strict --packager-layers=util,app --gen-packages-update-visibility-for=B -a . 2>&1

echo
echo "###### Running gen-packages=strict ######"

"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --sorbet-packages --gen-packages=strict --packager-layers=util,app . -a 2>&1

echo
echo "###### cat A/__package.rb ######"
cat A/__package.rb

echo
echo "###### cat B/__package.rb ######"
cat B/__package.rb

echo
echo "###### cat C/__package.rb ######"
cat C/__package.rb

echo
echo "###### cat D/__package.rb ######"
cat D/__package.rb

echo
echo "###### cat E/__package.rb ######"
cat E/__package.rb
