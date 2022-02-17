#!/bin/bash

set -euo pipefail
if [ "${CLEAN_BUILD:-}" != "" ] || [ "${PUBLISH_TO_RUBYGEMS:-}" != "" ]; then
  echo "Skipping publish, because this is a scheduled build."
  exit 0
fi

echo "--- setup"
apt-get update
apt-get install -yy curl jq rubygems file

git config --global user.email "sorbet+bot@stripe.com"
git config --global user.name "Sorbet build farm"

dryrun="1"
if [ "$BUILDKITE_BRANCH" == 'master' ]; then
    dryrun=""
fi

git_commit_count=$(git rev-list --count HEAD)
prefix="0.5"
release_version="$prefix.${git_commit_count}"
long_release_version="${release_version}.$(git log --format=%cd-%h --date=format:%Y%m%d%H%M%S -1)"

echo "--- Dowloading artifacts"
rm -rf release
rm -rf _out_
buildkite-agent artifact download "_out_/**/*" .

echo "--- releasing sorbet.run"

rm -rf sorbet.run
git clone git@github.com:sorbet/sorbet.run.git --single-branch --branch master
tar -xvf ./_out_/webasm/sorbet-wasm.tar ./sorbet-wasm.wasm ./sorbet-wasm.js
mv sorbet-wasm.wasm sorbet.run/docs
mv sorbet-wasm.js sorbet.run/docs
pushd sorbet.run/docs
git add sorbet-wasm.wasm sorbet-wasm.js
dirty=
git diff-index --quiet HEAD -- || dirty=1
if [ "$dirty" != "" ]; then
  echo "$BUILDKITE_COMMIT" > sha.html
  git add sha.html
  git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
  if [ "$dryrun" = "" ]; then
      git push
  fi
else
  echo "Nothing to update"
fi
popd
rm -rf sorbet.run

echo "--- releasing sorbet.org"
git fetch origin gh-pages
current_rev=$(git rev-parse HEAD)
git checkout gh-pages
# Remove all tracked files, but leave untracked files (like _out_) untouched
git rm -rf '*'
tar -xjf _out_/website/website.tar.bz2 .
git add .
git reset HEAD _out_
git reset HEAD sha.html
dirty=
git diff-index --quiet HEAD -- || dirty=1
if [ "$dirty" != "" ]; then
  echo "$BUILDKITE_COMMIT" > sha.html
  git add sha.html
  git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
  if [ "$dryrun" = "" ]; then
      git push origin gh-pages

      # For some reason, GitHub Pages won't automatically build for us on push
      # We have a ticket open with GitHub to investigate why.
      # For now, we trigger a build manually.
      curl \
        -X POST \
        --netrc \
        -H "Accept: application/vnd.github.mister-fantastic-preview+json" \
        "https://api.github.com/repos/sorbet/sorbet/pages/builds"
  fi
  echo "pushed an update"
else
  echo "nothing to update"
fi
git checkout -f "$current_rev"

echo "--- making a github release"
echo releasing "${long_release_version}"
git tag -f "${long_release_version}"
if [ "$dryrun" = "" ]; then
    git push origin "${long_release_version}"
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
files=()
while IFS='' read -r line; do files+=("$line"); done < <(find . -type f | sed 's#^./##')
backticks='```' # hack for bad Vim syntax highlighting definition
release_notes="To use Sorbet add this line to your Gemfile:
$backticks
gem 'sorbet', '$release_version', :group => :development
gem 'sorbet-runtime', '$release_version'
$backticks"
if [ "$dryrun" = "" ]; then
  echo "$release_notes" | ../.buildkite/tools/gh-release.sh sorbet/sorbet "${long_release_version}" -- "${files[@]}"
fi
popd

