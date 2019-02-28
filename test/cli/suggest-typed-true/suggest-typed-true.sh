#!/bin/bash

tmp="$(mktemp -d)"
cwd="$(pwd)"
cd "$tmp" || exit 1
separator() {
    echo -------------------------
}

cp "$cwd"/test/cli/suggest-typed-true/*.rb "$tmp/"

"$cwd/main/sorbet" --silence-dev-message --suggest-typed does-not-exist.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message --suggest-typed --typed=ruby does-not-exist.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message --suggest-typed --typed=true does-not-exist.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message -a --suggest-typed --typed=true does-not-exist.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message --suggest-typed --typed=true suggest-typed-true.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message -a --suggest-typed --typed=true suggest-typed-true.rb 2>&1
cat suggest-typed-true.rb
separator

cp "$cwd/test/cli/suggest-typed-true/suggest-typed-true.rb" "$tmp"
"$cwd/main/sorbet" --silence-dev-message --suggest-typed --typed=true suggest-typed-true.rb suggest-typed-with-error.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message --suggest-typed --typed=true empty.rb 2>&1
separator

rm -r "$tmp"
