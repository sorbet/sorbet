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

echo "--- Patching rubygems/package.rb"
# There's a bug in rubygems/package.rb where if there's a GZip reader error, in
# the process of reporting a good error saying what failed, there's a
# NoMethodError.
# https://github.com/rubygems/rubygems/pull/7539
package_rb="$HOME/.rbenv/versions/3.1.2/lib/ruby/3.1.0/rubygems/package.rb"
if [ -f "$package_rb" ]; then
  echo "Using sed to patch rubygems/package.rb"
  sed -i 's/@path = source.path/@path = source.is_a?(String) ? source : source.path/' "$package_rb"
else
  echo "Could not find rubygems/package.rb, skipping"
fi

publish_sorbet_static_gem() {
  gem_archive=$1
  platform=$2

  if gem list --remote rubygems.org --exact 'sorbet-static' | grep -q "${release_version}[^,]*${platform}"; then
    echo "$gem_archive already published."
    return
  fi

  # This is last so the exit code is used as the status code for with_backoff
  gem push --verbose "$gem_archive"
}

# Push the sorbet-static gems first, in case they fail. We don't want to end
# up in a weird state where 'sorbet' requires a pinned version of
# sorbet-static, but the sorbet-static gem push failed.
#
# (By failure here, we mean that RubyGems.org 502'd for some reason.)
for gem_archive in "_out_/gems/sorbet-static-$release_version"-*.gem; do
  if ! [[ "$gem_archive" =~ _out_/gems/sorbet-static-([^-]*)-([^.]*).gem ]]; then
    echo "Regex match failed. This should never happen."
    exit 1
  fi

  platform="${BASH_REMATCH[2]}"

  echo "Attempting to publish $gem_archive"
  with_backoff publish_sorbet_static_gem "$gem_archive" "$platform"
done

# Sometimes the 'gem push' times out, but after the connection dies, the server
# decides to finish publishing the gem. So we have to interleave 'gem list' and
# 'gem push' calls--it's not enough to just check whether it exists once.
publish_gem() {
  gem_name=$1
  gem_archive="_out_/gems/$gem_name-$release_version.gem"

  if gem list --remote rubygems.org --exact "$gem_name" | grep -q "$release_version"; then
    echo "$gem_archive already published."
    return
  fi

  # This is last so the exit code is used as the status code for with_backoff
  # TODO(jez) All this extra garbage is attempting to debug the root cause of https://github.com/rubygems/rubygems/pull/7539
  set +e
  gem push --verbose "$gem_archive"
  exit_code="$?"
  set -e
  if [ "$exit_code" -ne 0 ]; then
    mkdir -p _out_/corrupt
    cp "$gem_archive" _out_/corrupt
  fi
  return "$exit_code"
}

with_backoff publish_gem "sorbet-runtime"
with_backoff publish_gem "sorbet"
with_backoff publish_gem "sorbet-static-and-runtime"
