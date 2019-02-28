#!/bin/bash

tmp="$(mktemp -d)"
cwd="$(pwd)"
cd "$tmp" || exit 1
separator() {
    echo -------------------------
}

cp "$cwd"/test/cli/suggest-typed-true/*.rb "$tmp/"


"$cwd/main/sorbet" --silence-dev-message --suggest-typed --typed=false does-not-exist.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message --suggest-typed --typed=true does-not-exist.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message -a --suggest-typed does-not-exist.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message --suggest-typed suggest-typed-true.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message -a --suggest-typed suggest-typed-ignore.rb 2>&1
cat suggest-typed-ignore.rb
"$cwd/main/sorbet" --silence-dev-message --suggest-typed suggest-typed-ignore.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message -a --suggest-typed suggest-typed-false.rb 2>&1
cat suggest-typed-false.rb
"$cwd/main/sorbet" --silence-dev-message --suggest-typed suggest-typed-false.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message -a --suggest-typed suggest-typed-true.rb 2>&1
cat suggest-typed-true.rb
"$cwd/main/sorbet" --silence-dev-message --suggest-typed suggest-typed-true.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message -a --suggest-typed suggest-typed-strict.rb 2>&1
cat suggest-typed-strict.rb
"$cwd/main/sorbet" --silence-dev-message --suggest-typed suggest-typed-strict.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message -a --suggest-typed suggest-typed-strong.rb 2>&1
cat suggest-typed-strong.rb
"$cwd/main/sorbet" --silence-dev-message --suggest-typed suggest-typed-strong.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message --suggest-typed empty.rb 2>&1
separator

"$cwd/main/sorbet" --silence-dev-message -a --suggest-typed suggest-typed-with-too-low.rb 2>&1
cat suggest-typed-with-too-low.rb
separator

"$cwd/main/sorbet" --silence-dev-message --suggest-typed suggest-typed-with-too-low.rb 2>&1
separator

rm -r "$tmp"
