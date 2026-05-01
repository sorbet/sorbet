#!/bin/bash

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-gen-packages || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

echo "###### --gen-packages not allowed without --sorbet-packages ######"

"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --gen-packages --packager-layers=util,app -a . 2>&1

echo "###### --gen-packages not allowed with --lsp ######"

"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --lsp --sorbet-packages --gen-packages --packager-layers=util,app -a . 2>&1

echo "##################################"
echo "###### Non package directed ######"
echo "##################################"

echo
echo "###### Running gen-packages ######"

"$cwd/main/sorbet" --did-you-mean=false --max-threads=0 --silence-dev-message --sorbet-packages --gen-packages --packager-layers=util,app  --gen-packages-update-visibility-for=B --gen-packages-update-visibility-for=E --gen-packages-allow-relaxing-test-visibility -a . 2>&1

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
echo "###### cat D/ANestedPackage/__package.rb ######"
cat D/ANestedPackage/__package.rb

echo
echo "###### cat E/__package.rb ######"
cat E/__package.rb



echo "##############################"
echo "###### Package directed ######"
echo "##############################"

tmp="$(mktemp -d)"
cd "$cwd" || exit 1
cd test/cli/package-gen-packages || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

echo
echo "###### Running gen-packages ######"

"$cwd/main/sorbet" --max-threads=0 --did-you-mean=false --silence-dev-message --sorbet-packages --experimental-package-directed --gen-packages --packager-layers=util,app  --gen-packages-update-visibility-for=B --gen-packages-update-visibility-for=E --gen-packages-allow-relaxing-test-visibility -a . 2>&1

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
echo "###### cat D/ANestedPackage/__package.rb ######"
cat D/ANestedPackage/__package.rb

echo
echo "###### cat E/__package.rb ######"
cat E/__package.rb
