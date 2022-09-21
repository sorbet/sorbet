#!/bin/bash

set -euo pipefail

if [ "${PUBLISH_TO_RUBYGEMS:-}" == "" ]; then
  echo "Skipping because this build is not the nightly RubyGems scheduled build"
  exit 0
fi

git_commit_count=$(git rev-list --count HEAD)
prefix="0.5"
release_version="$prefix.${git_commit_count}"

echo "--- Dowloading artifacts"
rm -rf release
rm -rf _out_
buildkite-agent artifact download "_out_/**/*" .

echo "+++ Publishing to RubyGems.org"

mkdir -p "$HOME/.gem"
printf -- $'---\n:rubygems_api_key: %s\n' "$SORBET_RUBYGEMS_API_KEY" > "$HOME/.gem/credentials"
chmod 600 "$HOME/.gem/credentials"

source .buildkite/tools/with_backoff.sh

rbenv install --skip-existing

# Push the sorbet-static gems first, in case they fail. We don't want to end
# up in a weird state where 'sorbet' requires a pinned version of
# sorbet-static, but the sorbet-static gem push failed.
#
# (By failure here, we mean that RubyGems.org 502'd for some reason.)
for gem_archive in "_out_/gems/sorbet-static-$release_version"-*.gem; do
  echo "Attempting to publish $gem_archive"
  if [[ "$gem_archive" =~ _out_/gems/sorbet-static-([^-]*)-([^.]*).gem ]]; then
    platform="${BASH_REMATCH[2]}"
    if ! gem list --remote rubygems.org --exact 'sorbet-static' | grep -q "${release_version}[^,]*${platform}"; then
      with_backoff gem push --verbose "$gem_archive"
    else
      echo "$gem_archive already published."
    fi
  else
    echo "Regex match failed. This should never happen."
    exit 1
  fi
done

gem_archive="_out_/gems/sorbet-runtime-$release_version.gem"
echo "Attempting to publish $gem_archive"
if ! gem list --remote rubygems.org --exact 'sorbet-runtime' | grep -q "$release_version"; then
  with_backoff gem push --verbose "$gem_archive"
else
  echo "$gem_archive already published."
fi

gem_archive="_out_/gems/sorbet-$release_version.gem"
echo "Attempting to publish $gem_archive"
if ! gem list --remote rubygems.org --exact 'sorbet' | grep -q "$release_version"; then
  with_backoff gem push --verbose "$gem_archive"
else
  echo "$gem_archive already published."
fi

gem_archive="_out_/gems/sorbet-static-and-runtime-$release_version.gem"
echo "Attempting to publish $gem_archive"
if ! gem list --remote rubygems.org --exact 'sorbet-static-and-runtime' | grep -q "$release_version"; then
  with_backoff gem push --verbose "$gem_archive"
else
  echo "$gem_archive already published."
fi
