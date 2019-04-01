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
git_commit_count=$(git rev-list --count HEAD)
release_version="0.4.${git_commit_count}"
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet-runtime.gemspec
gem build sorbet-runtime.gemspec
popd

rm -rf _out_
mkdir -p _out_
cp gems/sorbet-runtime/sorbet-runtime-*.gem _out_/
