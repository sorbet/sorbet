#!/bin/bash

# Runs a single srb init test from gems/sorbet/test/snapshot/{partial,total}/*

set -euo pipefail
shopt -s dotglob

source "gems/sorbet/test/snapshot/logging.sh"

# ----- Option parsing -----

test_name=$1
archive_path=$2

if [[ "${test_name}" =~ partial/* ]]; then
  is_partial=1
else
  is_partial=
fi

info "├─ test_name:    ${test_name}"
info "├─ archive_path: ${archive_path}"
info "├─ is_partial:   ${is_partial:-0}"

# ----- Environment setup -----

repo_root=$PWD

test_dir="${repo_root}/gems/sorbet/test/snapshot/${test_name}"

# ----- Artifact validation -----

(
  cd "$test_dir"

  info "├─ Extracting test artifacts"
  tar -xvf $(basename "${archive_path}")

  # ----- Check out.log -----

  info "Checking output log"
  if [ "$is_partial" = "" ] || [ -f "expected/out.log" ]; then
    if ! diff -u "expected/out.log" "actual/out.log"; then
      error "├─ expected out.log did not match actual out.log"
      error "└─ see output above."
      exit 1
    fi
  fi

  # ----- Check err.log -----

  if [ "$is_partial" = "" ] || [ -f "expected/err.log" ]; then
    if ! diff -u "expected/err.log" "actual/err.log"; then
      error "├─ expected err.log did not match actual err.log"
      error "└─ see output above."
      exit 1
    fi
  fi

  # ----- Check sorbet/ -----

  diff_total() {
    info "├─ checking for total match"
    if ! diff -ur "expected/sorbet" "actual/sorbet"; then
      error "├─ expected sorbet/ folder did not match actual sorbet/ folder"
      error "└─ see output above."
      exit 1
    fi
  }

  diff_partial() {
    info "├─ checking for partial match"

    set +e
    diff -ur "expected/sorbet" "actual/sorbet" | \
      grep -vF "Only in actual" > "partial-diff.log"
    set -e

    # File exists and is non-empty
    if [ -s "partial-diff.log" ]; then
      cat "partial-diff.log"
      error "├─ expected sorbet/ folder did not match actual sorbet/ folder"
      error "└─ see output above."
      exit 1
    fi
  }

  if [ "$is_partial" = "" ]; then
    diff_total
  elif [ -d "expected/sorbet" ]; then
    diff_partial
  else
    # It's fine for a partial test to not have an expected dir.
    # It means the test only cares about the exit code of srb init.
    true
  fi

  success "└─ test passed."
)
