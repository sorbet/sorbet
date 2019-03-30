#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

bazel build //tools/toolchain/webasm-darwin:generate_em_cache
cp bazel-genfiles/tools/toolchain/webasm-darwin/em_cache.tar.gz tools/toolchain/webasm-darwin/em_cache_existing.tar.gz
