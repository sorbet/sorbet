#!/bin/bash

cwd="$(pwd)"

for mode in normal strict; do
  tmp="$(mktemp -d)"
  cp -RL test/cli/package-gen-packages-outer-package/outer "$tmp/"
  cd "$tmp" || exit 1

  echo "###### Running gen-packages=$mode ######"

  "$cwd/main/sorbet" --max-threads=0 --silence-dev-message --censor-for-snapshot-tests --sorbet-packages \
    --gen-packages="$mode" --packager-layers=util,app -a . 2>&1

  for pkg in outer outer/used outer/used/inner outer/unused outer/unused/inner outer/missing; do
    echo
    echo "###### cat $pkg/__package.rb ######"
    cat "$pkg/__package.rb"
  done
  echo

  cd "$cwd" || exit 1
  rm -rf "$tmp"
done
