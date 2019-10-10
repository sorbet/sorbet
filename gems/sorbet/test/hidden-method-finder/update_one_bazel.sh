#!/bin/bash

set -euo pipefail
shopt -s dotglob

# shellcheck disable=SC1090
source "gems/sorbet/test/hidden-method-finder/logging.sh"
hidden_path="$1"
test_name="$2"
ruby_version="$3"
expected="${ruby_version}_hidden.rbi.exp"

repo_root="$PWD"
test_dir="$repo_root/gems/sorbet/test/hidden-method-finder/$test_name"
# move to the test_dir

info "├─ hidden_methods_path: $hidden_path"
info "├─ test_name:           $test_name"
info "├─ test_dir:            $test_dir"

if ! diff "${expected}" "${repo_root}/${hidden_path}"; then
    attn "├─ updating ${expected}:"
    rm -f "${expected}"
    cp "${repo_root}/${hidden_path}" "${expected}"
fi

popd || fatal "Unable to leave test_dir"
