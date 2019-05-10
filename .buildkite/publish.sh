#!/bin/bash

set -euo pipefail
if [ "schedule" == "${BUILDKITE_SOURCE}" ]; then
  exit 0
fi

echo "--- setup"
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
dirty=
git diff-index --quiet HEAD -- || dirty=1
if [ -n "$dirty" ]; then
  echo "$BUILDKITE_COMMIT" > sha.html
  git add sha.html
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
current_rev=$(git rev-parse HEAD)
git checkout gh-pages
tar -xjf _out_/website/website.tar.bz2 .
git add .
git reset HEAD _out_
dirty=
git diff-index --quiet HEAD -- || dirty=1
if [ -n "$dirty" ]; then
  echo "$BUILDKITE_COMMIT" > sha.html
  git add sha.html
  git commit -m "Updated site - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
  if [ -z "$dryrun" ]; then
      git push origin gh-pages
  fi
  echo "pushed an update"
else
  echo "nothing to update"
fi
git checkout -f "$current_rev"

echo "--- releasing stripe.dev/sorbet-repo"
rm -rf sorbet-repo
git clone git@github.com:stripe/sorbet-repo.git
pushd sorbet-repo
git fetch origin gh-pages
git checkout gh-pages
mkdir -p super-secret-private-beta/gems/
cp -R ../_out_/gems/*.gem super-secret-private-beta/gems/
gem install builder
pushd super-secret-private-beta
gem generate_index
popd
git add super-secret-private-beta/
git commit -m "Updated gems - $(date -u +%Y-%m-%dT%H:%M:%S%z)"
if [ -z "$dryrun" ]; then
    git push origin gh-pages
fi
popd

echo "--- making a github release"
git_commit_count=$(git rev-list --count HEAD)
prefix="0.4"
release_version="v$prefix.$git_commit_count.$(git log --format=%cd-%h --date=format:%Y%m%d%H%M%S -1)"
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
files=()
while IFS='' read -r line; do files+=("$line"); done < <(find . -type f | sed 's#^./##')
release_notes="To use Sorbet add this line to your Gemfile:
\`\`\`
source 'https://stripe.dev/sorbet-repo/super-secret-private-beta/' do
  gem 'sorbet', '$prefix.$git_commit_count'
end
\`\`\`"
if [ -z "$dryrun" ]; then
    echo "$release_notes" | ../.buildkite/gh-release.sh stripe/sorbet "${release_version}" -- "${files[@]}"
fi
popd
