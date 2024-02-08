#!/bin/bash

set -eo pipefail

pushd gems/sorbet-static-and-runtime

echo "--- setup :ruby:"
eval "$(rbenv init -)"

# Uses the version in .ruby-version
rbenv rehash
ruby --version

echo "--- build"
git_commit_count=$(git rev-list --count HEAD)
release_version="0.5.${git_commit_count}"
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet-static-and-runtime.gemspec
gem build sorbet-static-and-runtime.gemspec

popd

rm -rf _out_
mkdir -p _out_/gems/
cp gems/sorbet-static-and-runtime/sorbet-static-and-runtime-*.gem _out_/gems/
