#!/bin/bash

set -euo pipefail

pushd website
yarn
yarn build
tar -cjf website.tar.bz2 -C build/sorbet .
popd

mkdir _out_/website
cp website/website.tar.bz2 _out_/website
