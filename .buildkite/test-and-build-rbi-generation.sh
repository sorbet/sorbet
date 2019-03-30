#!/bin/bash

set -euo pipefail

pushd rbi-generation
gem build sorbet-rbi-generation.gemspec
popd

rm -rf _out_
mkdir -p _out_
cp rbi-generation/sorbet-rbi-generation-*.gem _out_/
