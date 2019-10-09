#!/usr/bin/env bash

set -euo pipefail
shopt -s dotglob

# shellcheck disable=SC1090
source "gems/sorbet/test/hidden-method-finder/logging.sh"
hidden_path="$1"
test_name="$2"

repo_root="$PWD"
# move to the test_dir
pushd "$repo_root/gems/sorbet/test/hidden-method-finder/$test_name"

if ! diff -ur "hidden.rbi.exp" "${repo_root}/${hidden_path}"; then
  attn "├─ updating hidden.rbi.exp:"
  rm -f "hidden.rbi.exp"
  cp "${repo_root}/${hidden_path}" "hidden.rbi.exp"
fi
