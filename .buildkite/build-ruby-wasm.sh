#!/bin/bash

set -eo pipefail

mkdir -p /tmp/ruby-wasm
pushd /tmp/ruby-wasm

echo "--- setup :ruby:"
eval "$(rbenv init -)"

runtime_version=3.1.2
rbenv install --skip-existing "$runtime_version"

echo "3.1.2" > .ruby-version

bundle init
bundle add ruby_wasm
bundle add sorbet-runtime
bundle exec rbwasm build -o ruby.wasm

rm -rf _out_
mkdir -p _out_/webasm_ruby
cp ruby.wasm _out_/webasm_ruby/ruby.wasm
