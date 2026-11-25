#!/bin/bash

cwd="$(pwd)"

sorbet_args=(--did-you-mean=false --max-threads=0 --censor-for-snapshot-tests --silence-dev-message --sorbet-packages)

tmp="$(mktemp -d)"
cd test/cli/package-gen-packages-alias-imports || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

echo "##################################"
echo "###### Non package directed ######"
echo "##################################"

echo
echo "###### Running gen-packages (first run) ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" --gen-packages=strict -a . 2>&1

echo
echo "###### cat consumer/__package.rb ######"
cat consumer/__package.rb

echo
echo "###### cat target_only_consumer/__package.rb ######"
cat target_only_consumer/__package.rb

echo
echo "###### Typechecking ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" . 2>&1

echo
echo "###### Running gen-packages (second run) ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" --gen-packages=strict -a . 2>&1

echo
echo "###### cat consumer/__package.rb ######"
cat consumer/__package.rb

echo
echo "###### cat target_only_consumer/__package.rb ######"
cat target_only_consumer/__package.rb

echo
echo "###### Typechecking ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" . 2>&1

echo "##############################"
echo "###### Package directed ######"
echo "##############################"

tmp="$(mktemp -d)"
cd "$cwd" || exit 1
cd test/cli/package-gen-packages-alias-imports || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

echo
echo "###### Running gen-packages (first run) ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" --experimental-package-directed --gen-packages=strict -a . 2>&1

echo
echo "###### cat consumer/__package.rb ######"
cat consumer/__package.rb

echo
echo "###### cat target_only_consumer/__package.rb ######"
cat target_only_consumer/__package.rb

echo
echo "###### Typechecking ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" --experimental-package-directed . 2>&1

echo
echo "###### Running gen-packages (second run) ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" --experimental-package-directed --gen-packages=strict -a . 2>&1

echo
echo "###### cat consumer/__package.rb ######"
cat consumer/__package.rb

echo
echo "###### cat target_only_consumer/__package.rb ######"
cat target_only_consumer/__package.rb

echo
echo "###### Typechecking ######"

"$cwd/main/sorbet" "${sorbet_args[@]}" --experimental-package-directed . 2>&1
