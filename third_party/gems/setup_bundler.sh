#!/bin/bash

# A template script to unpack the internals of the bundler gem into a local tree
# that looks like the site_ruby tree that the tracepoint tracer is expecting to
# find

set -euo pipefail

ruby_versions=("$@")

cd "{{bundler}}"

pushd extracted
  tar xvf data.tar.gz
  rm -f data.tar.gz metadata.gz checksums.yaml.gz
popd

for RUBY_VERSION in "${ruby_versions[@]}"; do
  TARGET="lib/ruby/site_ruby/${RUBY_VERSION}"
  mkdir -p "$TARGET"
  cp -a extracted/lib/* "$TARGET"
done

mkdir -p "{{site_bin}}"
mv extracted/exe/bundle "{{site_bin}}"
mv extracted/exe/bundler "{{site_bin}}"
