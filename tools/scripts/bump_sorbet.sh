#!/usr/bin/env bash

set -euo pipefail

branch="${1:-master}"
sorbet_repo="${2:-HOME/stripe/sorbet/.git}"

git --git-dir "$sorbet_repo" fetch --quiet origin "$branch"
new_sorbet_version="$(git --git-dir="$sorbet_repo" rev-parse "origin/$branch")"
new_shallow_since="$(git --git-dir="$sorbet_repo" log -n 1 --pretty=format:"%cd" --date=raw "origin/$branch")"

sorbet_version_bzl="third_party/sorbet_version.bzl"
sed -i.bak -e "s/SORBET_VERSION = \"[^\"]*\"/SORBET_VERSION = \"$new_sorbet_version\"/" "$sorbet_version_bzl"
sed -i.bak -e "s/SORBET_SHALLOW_SINCE = \"[^\"]*\"/SORBET_SHALLOW_SINCE = \"$new_shallow_since\"/" "$sorbet_version_bzl"
rm "$sorbet_version_bzl.bak"
