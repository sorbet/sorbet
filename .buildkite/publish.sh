#!/bin/bash

set -euo pipefail
echo "--- setup"
apt-get update
apt-get install -yy curl jq

git config --global user.email "sorbet+bot@stripe.com"
git config --global user.name "Sorbet build farm"

dryrun="1"
if [ "$BUILDKITE_BRANCH" == 'master' ]; then
    dryrun=""
fi

echo "--- Dowloading artifacts"
rm -rf release
rm -rf _out_
buildkite-agent artifact download "_out_/**/*" .

echo "--- releasing sorbet.run"

rm -rf sorbet.run
git clone git@github.com:stripe/sorbet.run.git
tar -xvf ./_out_/webasm/sorbet-wasm.tar ./sorbet-wasm.wasm ./sorbet-wasm.js
mv sorbet-wasm.wasm sorbet.run/docs
mv sorbet-wasm.js sorbet.run/docs
pushd sorbet.run/docs
git add sorbet-wasm.wasm sorbet-wasm.js
dirty=1
git diff-index --quiet HEAD -- || dirty=0
if [ "$dirty" -ne 0 ]; then
  echo "$BUILDKITE_COMMIT" > docs/sha.html
  git add docs/sha.html
  git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
  if [ -z "$dryrun" ]; then
      git push
  fi
else
  echo "Nothing to update"
fi
popd

echo "--- releasing stripe.dev/sorbet"
git fetch origin gh-pages
git branch -D gh-pages || true
git checkout gh-pages
tar -xjf _out_/website/website.tar.bz2 -C super-secret-private-beta .
git add super-secret-private-beta
dirty=1
git diff-index --quiet HEAD -- || dirty=0
if [ "$dirty" -ne 0 ]; then
  echo "$BUILDKITE_COMMIT" > super-secret-private-beta/sha.html
  git add super-secret-private-beta/sha.html
  git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
  if [ -z "$dryrun" ]; then
      git push origin gh-pages
  fi
  echo "pushed an update"
else
  echo "nothing to update"
fi

echo "--- releasing stripe.dev/sorbet/repo"
mkdir -p repo/super-secret-private-beta/gems/
cp -R _out_/gems/*.gem repo/super-secret-private-beta/gems/
pushd repo/super-secret-private-beta
gem install builder
gem generate_index
popd
git add repo/super-secret-private-beta/
git commit -m "Updated gems - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
if [ -z "$dryrun" ]; then
    git push origin gh-pages
fi

echo "--- making a github release"
git_commit_count=$(git rev-list --count HEAD)
release_version="v0.4.${git_commit_count}.$(git log --format=%cd-%h --date=format:%Y%m%d%H%M%S -1)"
echo releasing "${release_version}"
git tag -f "${release_version}"
if [ -z "$dryrun" ]; then
    git push origin "${release_version}"
fi

mkdir release
cp -R _out_/* release/
mv release/gems/* release
rmdir release/gems
rm release/website/website.tar.bz2
rmdir release/website
rm release/webasm/sorbet-wasm.tar
rmdir release/webasm

pushd release
ls -al
if [ -z "$dryrun" ]; then
    # shellcheck disable=SC2035
    find . -type f -print0 | xargs -0 ../.buildkite/gh-release.sh stripe/sorbet "${release_version}" --
fi
popd
