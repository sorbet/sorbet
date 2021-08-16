#!/bin/bash

set -euo pipefail

sorbet_repo_root="$(cd "$(dirname "$0")"/../.. ; pwd)"

# Base tag to use as the base for the patch.
base_tag="v2_7_2"

# Branch to use to generate the patch.
patch_branch="sorbet_ruby_2_7"

# Filename for the patch in the repo.
patch_filename="sorbet_ruby_2_7.patch"

# We'll let GitHub generate the patch for us.
patch_source_url="https://github.com/sorbet/ruby/compare/${base_tag}...${patch_branch}.patch"

echo "Fetching new patch file from GitHub..."
curl -o "$sorbet_repo_root/third_party/ruby/${patch_filename}" "$patch_source_url"
echo "Done!"
