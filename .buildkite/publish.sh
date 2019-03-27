#!/bin/bash

set -euo pipefail

echo "--- Dowloading artifacts"
mkdir release

buildkite-agent artifact download "_out_/**/*" .

echo "--- making a github release"
git_rev=$(git rev-parse HEAD)
git_commit_count=$(git rev-list --count HEAD)
release_version="0.1.${git_commit_count}.${git_rev}"
.buildkite/gh-release.sh create stripe/sorbet "${release_version}" "$BUILDKITE_BRANCH" 

echo "--- releasing stripe.dev/sorbet"
git pull
git checkout gh-pages
tar -xjf _out_/website/website.tar.bz2 -C super-secret-private-beta .
git add super-secret-private-beta
git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
git push github.com gh-pages

echo "--- releasing sorbet.run"
rm -rf sorbet.run
git clone git@github.com:stripe/sorbet.run.git
tar -C sorbet.run/docs -xvf ./_out_/webasm/sorbet-wasm.tar sorbet-wasm.wasm sorbet-wasm.js
git rev-parse HEAD > sorbet.run/docs/sha.html
cd sorbet.run
git add sorbet-wasm.wasm sorbet-wasm.js
git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
git push
