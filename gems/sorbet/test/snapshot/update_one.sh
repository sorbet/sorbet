#!/usr/bin/env bash

set -euo pipefail
shopt -s dotglob

# shellcheck source-path=SCRIPTDIR/../../../..
source "gems/sorbet/test/snapshot/logging.sh"
# shellcheck source-path=SCRIPTDIR/../../../..
source "gems/sorbet/test/snapshot/validate_utils.sh"

setup_validate_env "$@"

# ----- Update sorbet/ -----

diff_total() {
  if ! diff -ur "expected/sorbet" "$actual/sorbet"; then
    attn "├─ updating expected/sorbet (total):"
    rm -rf "$test_dir/expected/sorbet"
    mkdir -p "$test_dir/expected"
    cp -r "$actual/sorbet" "$test_dir/expected"
  fi
}

diff_partial() {
  set +e
  diff -ur "expected" "$actual" | \
    grep -vF "Only in actual" \
    > "$actual/partial-diff.log"
  set -e

  # File exists and is non-empty
  if [ -s "$actual/partial-diff.log" ]; then
    attn "├─ updating expected/sorbet (partial):"

    find "$test_dir/expected/sorbet" -print0 | while IFS= read -r -d '' expected_path; do
      path_suffix="${expected_path#"$test_dir"/expected/sorbet}"
      actual_path="$actual/sorbet$path_suffix"

      # Only ever update existing files, never grow this partial snapshot.
      if [ -d "$expected_path" ]; then
        if ! [ -d "$actual_path" ]; then
          info "├─ removing dir: $expected_path"
          rm -rfv "$expected_path"
        fi
      else
        if [ -f "$actual_path" ]; then
          info "├─ updating file: $expected_path"
          cp -v "$actual_path" "$expected_path"
        else
          info "├─ removing file: $expected_path"
          rm -fv "$expected_path"
        fi
      fi
    done
  fi
}

if [ -d expected/sorbet ]; then
  if [ "$is_partial" = "" ]; then
    info "├─ Performing total diff"
    diff_total
  else
    info "├─ Performing partial diff"

    # we know that there's an expected dir, as that is a pre-requesite for this
    # test to be run by bazel
    diff_partial
  fi
else
  attn "├─ missing expected/sorbet directory, skipping update"
fi

# ----- Update logs -----
if [ -f "expected/err.log" ]; then
  info "├─ updating err.log"
  cp "$actual/err.log" expected/err.log
fi

if [ -f "expected/out.log" ]; then
  info "├─ updating out.log"
  cp "$actual/out.log" expected/out.log
fi

cleanup_validation
