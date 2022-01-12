#!/bin/bash

set -eo pipefail

pushd gems/sorbet-static-and-runtime

echo "--- setup :ruby:"
eval "$(rbenv init -)"

runtime_versions=(2.6.3 2.7.2)

for runtime_version in "${runtime_versions[@]}"; do
  rbenv install --skip-existing "$runtime_version"
  rbenv shell "$runtime_version"
done

echo "--- build"
git_commit_count=$(git rev-list --count HEAD)
release_version="0.5.${git_commit_count}"
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet-static-and-runtime.gemspec
gem build sorbet-static-and-runtime.gemspec

popd

rm -rf _out_
mkdir -p _out_/gems/
cp gems/sorbet-static-and-runtime/sorbet-static-and-runtime-*.gem _out_/gems/
