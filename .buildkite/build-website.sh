#!/bin/bash

set -euo pipefail

pushd website
yarn
yarn build
tar -cjf website.tar.bz2 -C build/sorbet .
popd

rm -rf _out_
mkdir -p _out_/website
cp website/website.tar.bz2 _out_/website
