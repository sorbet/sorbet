#!/bin/bash

set -euo pipefail

pushd website
yarn
yarn build
tar -cf website.tar.gz -C build/sorbet .
popd

rm -rf _out_
mkdir -p _out_/website
cp website/website.tar.gz _out_/website
