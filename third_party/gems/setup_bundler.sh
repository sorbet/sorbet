#!/bin/bash

# A template script to unpack the internals of the bundler gem into a local tree
# that looks like the site_ruby tree that the tracepoint tracer is expecting to
# find

set -euo pipefail

cd "%{bundler}"

pushd extracted
  tar xvf data.tar.gz
  rm -f data.tar.gz metadata.gz checksums.yaml.gz
popd

mkdir -p "%{site_ruby}"
mv extracted/lib "%{site_ruby}"

mkdir -p "%{site_bin}"
mv extracted/exe/bundle "%{site_bin}"
mv extracted/exe/bundler "%{site_bin}"
