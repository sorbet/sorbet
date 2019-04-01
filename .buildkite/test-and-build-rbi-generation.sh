#!/bin/bash

set -euo pipefail

pushd rbi-generation
git_commit_count=$(git rev-list --count HEAD)
release_version="0.4.${git_commit_count}"
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet-rbi-generation.gemspec
gem build sorbet-rbi-generation.gemspec
popd

rm -rf _out_
mkdir -p _out_
cp rbi-generation/sorbet-rbi-generation-*.gem _out_/
