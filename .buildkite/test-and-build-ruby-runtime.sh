#!/bin/bash

set -eo pipefail

pushd runtime

echo "--- setup :ruby:"
eval "$(rbenv init -)"
rbenv shell 2.4.3
bundle install --path vendor/bundle

echo "+++ tests"
bundle exec rake test

echo "--- build"
gem build sorbet-runtime.gemspec
popd

rm -rf _out_
mkdir -p _out_/gems
cp runtime/sorbet-runtime-*.gem _out_/gems
