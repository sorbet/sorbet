#!/bin/bash

set -euo pipefail

sorbet_repo_root="$(cd "$(dirname "$0")"/../.. ; pwd)"

# Base tag to use as the base for the patch.
base_tag="v2.1.2"

# Branch to use to generate the patch.
patch_branch="sorbet_ruby_bundler"

# Filename for the patch in the repo.
patch_filename="sorbet_ruby_bundler.patch"

# We'll let GitHub generate the patch for us.
patch_source_url="https://github.com/sorbet/bundler/compare/${base_tag}...${patch_branch}.patch"

echo "Fetching new patch file from GitHub..."
outfile="$sorbet_repo_root/third_party/ruby/${patch_filename}"
curl -o "$outfile" "$patch_source_url"

# We need to patch the paths in the generated patch, so that we can apply it with 'patch -p1' in the bazel build.
sed -i'' -e 's# \([ab]\)/lib/bundler# \1/lib/ruby/site_ruby/2.7.0/bundler#g' "$outfile"

echo "Done!"
