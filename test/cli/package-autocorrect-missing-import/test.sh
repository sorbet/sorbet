#!/bin/bash

# TODO(neil): this test is slow because it invokes sorbet a bunch of times. Combine all the invokations into one sorbet invocation.

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-autocorrect-missing-import || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --packager-layers=lib,app --max-threads=0 other other_test use_other_package 2>&1

cat use_other_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --packager-layers=lib,app --max-threads=0 app_package app_package_test use_app_package 2>&1

cat use_app_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --packager-layers=lib,app --max-threads=0 false_package false_package_test use_false_package 2>&1

cat use_false_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --packager-layers=lib,app --max-threads=0 false_and_app_package false_and_app_package_test use_false_and_app_package 2>&1

cat use_false_and_app_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --packager-layers=lib,app --max-threads=0 cycle_package cycle_package_test use_cycle_package 2>&1

cat use_cycle_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --packager-layers=lib,app --max-threads=0 app_cycle_package app_cycle_package_test use_app_cycle_package 2>&1

cat use_app_cycle_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --packager-layers=lib,app --max-threads=0 false_cycle_package false_cycle_package_test use_false_cycle_package 2>&1

cat use_false_cycle_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --packager-layers=lib,app --max-threads=0 app_false_cycle_package app_false_cycle_package_test use_app_false_cycle_package 2>&1

cat use_app_false_cycle_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --packager-layers=lib,app --max-threads=0 visible_to_package use_visible_to_package 2>&1

cat use_visible_to_package/__package.rb

rm -rf "$tmp"

exit 1
