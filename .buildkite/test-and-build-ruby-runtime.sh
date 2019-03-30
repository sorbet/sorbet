#!/bin/bash

set -eo pipefail

pushd gems/sorbet-runtime

echo "--- setup :ruby:"
eval "$(rbenv init -)"
rbenv shell 2.4.3
rbenv exec bundle install --path vendor/bundle

echo "+++ tests"
rbenv exec bundle exec rake test

echo "--- build"
gem build sorbet-runtime.gemspec
popd

rm -rf _out_
mkdir -p _out_
cp gems/sorbet-runtime/sorbet-runtime-*.gem _out_/
