#!/bin/bash

set -euo pipefail

pushd runtime
bundler install --deployment
bundle exec rake test
gem build sorbet-runtime.gemspec
popd

rm -rf _out_
mkdir -p _out_/gems
cp runtime/sorbet-runtime-*.gem _out_/gems
