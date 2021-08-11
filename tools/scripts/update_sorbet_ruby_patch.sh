#!/bin/bash

set -euo pipefail

sorbet_repo_root="$(cd "$(dirname "$0")"/../.. ; pwd)"

# Base tag to use as the base for the patch.
base_tag="v2_7_2"

# Branch to use to generate the patch.
patch_branch="sorbet_ruby_2_7"

# Make sure we have a copy of the sorbet_ruby repo.
if [ -z "${SORBET_RUBY_REPO_ROOT:-}" ]; then
  export SORBET_RUBY_REPO_ROOT="$HOME/stripe/sorbet_ruby"
fi

if [ ! -d "$SORBET_RUBY_REPO_ROOT/.git" ]; then
  if [ ! -e "$SORBET_RUBY_REPO_ROOT" ]; then
    echo "$SORBET_RUBY_REPO_ROOT does not exist. Will clone from https://github.com/sorbet/ruby."

    git clone https://github.com/sorbet/ruby "$SORBET_RUBY_REPO_ROOT"
  else
    echo "$SORBET_RUBY_REPO_ROOT exists but is not a git repository. Halting." >&2
    exit 1
  fi
fi

cd "$SORBET_RUBY_REPO_ROOT"

# Fetch remote refs for the base tag and patch branch.
echo "Fetching base tag $base_tag and patch branch $patch_branch in $SORBET_RUBY_REPO_ROOT..."
git fetch origin "$base_tag" "$patch_branch"

# Regenerate the .patch file.
echo "Regenerating sorbet_ruby_2_7.patch..."
git diff --no-prefix "$base_tag" "origin/$patch_branch" > "$sorbet_repo_root/third_party/ruby/sorbet_ruby_2_7.patch"

echo "Done!"
