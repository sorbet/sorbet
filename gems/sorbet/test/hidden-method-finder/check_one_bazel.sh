#!/bin/bash

set -euo pipefail
shopt -s dotglob

source "gems/sorbet/test/hidden-method-finder/logging.sh"
hidden_path="$1"
test_name="$2"
ruby_version="$3"

repo_root="$PWD"
# move to the test_dir
pushd "$repo_root/gems/sorbet/test/hidden-method-finder/$test_name"

# ----- Artifact validation -----

info "├─ checking for total hidden methods match"
if ! diff -u "./${ruby_version}_hidden.rbi.exp" "${repo_root}/${hidden_path}"; then
  error "├─ expected hidden.rbi did not match actual hidden.rbi"
  error "└─ see output above."
  exit 1
fi

# cleanup_validation
popd || fatal "Unable to leave test_dir"
