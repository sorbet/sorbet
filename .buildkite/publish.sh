#!/bin/bash

set -euo pipefail
if [ "${CLEAN_BUILD:-}" != "" ] || [ "${PUBLISH_TO_RUBYGEMS:-}" != "" ]; then
  echo "Skipping publish, because this is a scheduled build."
  exit 0
fi

echo "--- setup"

curl -fsSL https://deb.nodesource.com/gpgkey/nodesource.gpg.key | apt-key add -
echo "deb https://deb.nodesource.com/node_16.x focal main" | tee /etc/apt/sources.list.d/nodesource.list
echo "deb-src https://deb.nodesource.com/node_16.x focal main" | tee -a /etc/apt/sources.list.d/nodesource.list
curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add -
echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list

apt-get update
apt-get install -yy curl jq rubygems file nodejs yarn

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

echo "--- Downloading artifacts"
rm -rf release
rm -rf _out_
buildkite-agent artifact download "_out_/**/*" .

if [ "$dryrun" = "" ]; then
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
    git push
  else
    echo "Nothing to update"
  fi
  popd
  rm -rf sorbet.run
fi

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

echo "--- releasing VS Code extension"
pushd vscode_extension

extension_release_version="$(jq --raw-output '.version' package.json)"
echo "releasing $extension_release_version"

# (progress bar messes with Buildkite timestamps)
yarn install --no-progress

if node_modules/.bin/vsce show --json sorbet.sorbet-vscode-extension | \
    jq --raw-output '.versions[] | .version' | \
    grep -qFx "$extension_release_version"; then
  echo "... $extension_release_version is already published"
  node_modules/.bin/vsce show sorbet.sorbet-vscode-extension
elif [ "$dryrun" = "" ]; then
  echo "... starting publish"
  vsce_publish_output="$(mktemp)"
  trap 'rm -rf "$vsce_publish_output"' EXIT

  # Reads from the VSCE_PAT variable
  if ! node_modules/.bin/vsce publish --packagePath "../_out_/vscode_extension/sorbet.vsix" < /dev/null > "$vsce_publish_output" 2>&1; then
    # It can take 4+ minutes for the marketplace to "verify" our extension
    # before it shows up as published according to `vsce show`.
    cat "$vsce_publish_output"
    if grep -qF 'Version number cannot be the same'; then
      echo "... $extension_release_version is already published"
    else
      exit 1
    fi
  else
    echo "... published version $extension_release_version"
  fi
else
  echo "... skipping extension publish for dryrun."
fi
popd
