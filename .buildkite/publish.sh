#!/bin/bash

set -euo pipefail
echo "--- setup"
apt-get update
apt-get install -yy curl jq

echo "--- Dowloading artifacts"
rm -rf release
mkdir release

buildkite-agent artifact download "_out_/**/*" .
cp -R _out_/* release/

echo "--- making a github release"
git_rev=$(git rev-parse HEAD)
git_commit_count=$(git rev-list --count HEAD)
release_version="v0.4.${git_commit_count}.$(git log --format=%cd-%h --date=format:%Y%m%d%H%M%S -1)"
echo releasing "${release_version}"
git tag -f "${release_version}"
git push origin "${release_version}"
pushd release
../.buildkite/gh-release.sh stripe/sorbet "${release_version}" -- $(find * -type f -print)
popd

echo "--- releasing stripe.dev/sorbet"
git fetch origin gh-pages
git branch -D gh-pages || true
git checkout gh-pages
tar -xjf _out_/website/website.tar.bz2 -C super-secret-private-beta .
git add super-secret-private-beta
git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
git push origin gh-pages

echo "--- releasing sorbet.run"
rm -rf sorbet.run
git clone git@github.com:stripe/sorbet.run.git
tar -C sorbet.run/docs -xvf ./_out_/webasm/sorbet-wasm.tar sorbet-wasm.wasm sorbet-wasm.js
git rev-parse HEAD > sorbet.run/docs/sha.html
cd sorbet.run
git add sorbet-wasm.wasm sorbet-wasm.js
git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
git push
