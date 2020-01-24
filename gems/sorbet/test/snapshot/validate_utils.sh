#!/bin/bash

# Parses args assuming the fixed format:
# 1 = archive location
# 2 = test_path, relative to gems/sorbet/test/snapshot
#
# Sets the variables:
#   repo_root    = the repo root
#   archive_path = the value of $1
#   test_name    = the value of $2
#   is_partial   = 1 when the test is a partial test
#   test_dir     = the test directory within the repository
#   update_path  = the expected directory within test_dir
#
# Changes to the test_dir, and extracts the archive
setup_validate_env() {
  repo_root=$PWD

  archive_path=$1
  test_name=$2

  if [[ "${test_name}" =~ partial/* ]]; then
    is_partial=1
  else
    is_partial=
  fi

  test_dir="$repo_root/gems/sorbet/test/snapshot/$test_name"

  update_path="$test_dir/expected"

  info "├─ archive_path: $archive_path"
  info "├─ update_path:  $update_path"
  info "├─ test_name:    $test_name"
  info "├─ is_partial:   ${is_partial:-}"
  info "├─ test_dir:     $test_dir"

  pushd "$test_dir" || fatal "Unable to change to test_dir"

  actual="$(mktemp -d)"

  info "├─ Extracting test artifacts"
  attn "├─ $actual"
  tar -xvf "$(basename "${archive_path}")" -C "$actual"

  export actual
}

# Leave the test directory that was setup by setup_validate_env, and print a
# success message.
cleanup_validation() {

  # Cleanup the extracted run results
  rm -rf "$actual"

  popd || fatal "Unable to leave test_dir"
  success "└─ test passed."
}
