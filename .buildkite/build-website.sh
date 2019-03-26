#!/bin/bash

set -euo pipefail

pushd website
yarn
yarn build
cp -cjf website.tar.bz2 -C website/build/sorbet .
popd

mkdir _out_/website
cp website/website.tar.bz2 _out_/website
