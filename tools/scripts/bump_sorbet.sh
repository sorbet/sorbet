#!/usr/bin/env bash

set -euo pipefail

# ---- Option parsing ----
branch="${1:-master}"
sorbet_repo="${2:-$HOME/stripe/sorbet}"

# Fetch the sha for the branch given
fetch_branch_sha() {
  git --git-dir="$sorbet_repo/.git" fetch --quiet origin "$1"
  git --git-dir="$sorbet_repo/.git" rev-parse "origin/$1"
}

# Compute the sha256 of the archive that corresponds to the given commit ref
compute_archive_sha256() {
  curl -sL "https://github.com/sorbet/sorbet/archive/$1.zip" | \
    sha256sum | \
    cut -d " " -f 1
}

new_sorbet_version="$(fetch_branch_sha "$branch")"
new_sha256="$(compute_archive_sha256 "$new_sorbet_version")"

sorbet_version_bzl="third_party/sorbet_version.bzl"
sed -i.bak -e "s/SORBET_VERSION = \"[^\"]*\"/SORBET_VERSION = \"$new_sorbet_version\"/" "$sorbet_version_bzl"
sed -i.bak -e "s/SORBET_SHA256 = \"[^\"]*\"/SORBET_SHA256 = \"$new_sha256\"/" "$sorbet_version_bzl"
rm "$sorbet_version_bzl.bak"
