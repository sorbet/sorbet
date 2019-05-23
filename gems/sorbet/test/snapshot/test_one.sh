#!/usr/bin/env bash

set -euo pipefail
shopt -s dotglob

cd "$(dirname "${BASH_SOURCE[0]}")/../.." || exit 1
root_dir="$PWD"

# shellcheck disable=SC1091
source "test/snapshot/logging.sh"

# ----- Option parsing -----

usage() {
  echo
  echo "Usage:"
  echo "  $0 <test_dir> [options]"
  echo
  echo "Arguments:"
  echo "  <test_dir>   relative path of the snapshot to test"
  echo
  echo "Options:"
  echo "  --verbose    be more verbose than just whether it errored"
  echo "  --update     if there is a failure, update the expected file(s) and keep going"
}

if [[ $# -lt 1 ]]; then
  error "Missing test."
  usage
  exit 1
elif [[ "$1" =~ test/snapshot/partial* ]]; then
  test_dir="$1"
  is_partial=1
elif [[ "$1" =~ test/snapshot/total* ]]; then
  test_dir="$1"
  is_partial=
else
  error "Expected test_dir to match test/snapshot/(total|partial)/*. Got: $3"
  usage
  exit 1
fi
shift

VERBOSE=
UPDATE=
while [[ $# -gt 0 ]]; do
  case $1 in
    --verbose)
      VERBOSE="--verbose"
      shift
      ;;
    --update)
      UPDATE="--update"
      shift
      ;;
    -*)
      echo
      error "Unrecognized option: '$1'"
      usage
      exit 1
      ;;
    *)
      echo
      error "Unrecognized positional arg: '$1'"
      usage
      exit 1
      ;;
  esac
done


# ----- Stage the test sandbox directory -----

info "Running test:  $0 $test_dir $VERBOSE $UPDATE"

actual="$(mktemp -d)"

if [ -n "$VERBOSE" ]; then
  info "├─ PWD:       $PWD"
  info "├─ test_dir:  $test_dir"
  info "├─ actual:    $actual"
fi

srb="$root_dir/bin/srb"

if [ -z "${SRB_SORBET_EXE:-}" ]; then
  SRB_SORBET_EXE="$(realpath "$root_dir/../../bazel-bin/main/sorbet")"
fi
export SRB_SORBET_EXE

if ! [ -f "$SRB_SORBET_EXE" ]; then
  error "└─ could not find sorbet executable ($SRB_SORBET_EXE)"
  exit 1
fi

if ! [ -x "$SRB_SORBET_EXE" ]; then
  error "└─ sorbet executable has wrong permissions ($SRB_SORBET_EXE)"
  exit 1
fi

if ! [ -d "$test_dir" ]; then
  error "└─ test_dir doesn't exist: $test_dir"
  exit 1
fi

if ! [ -d "$test_dir/src" ]; then
  error "└─ each test must have a src/ dirctory: $test_dir/src"
  exit 1
fi

if ! [ -f "$test_dir/src/Gemfile" ]; then
  error "└─ each test must have src/Gemfile: $test_dir/src/Gemfile"
  exit 1
fi

if ! [ -f "$test_dir/src/Gemfile.lock" ]; then
  error "├─ each test must have src/Gemfile.lock: $test_dir/src/Gemfile.lock"
  if [ -z "$UPDATE" ]; then
    warn "└─ re-run with --update to create it."
    exit 1
  else
    warn "└─ running 'bundle install' to create it"
    (
      cd "$test_dir/src"
      bundle install
    )
  fi
fi

# TODO: make these files recordable
if [ -d "$test_dir/expected/sorbet/rbi/hidden-definitions" ]; then
  error "├─ hidden-definitions are not currently testable."

  if [ -z "$UPDATE" ]; then
    warn "└─ please remove: $test_dir/expected/sorbet/rbi/hidden-definitions"
    exit 1
  else
    warn "├─ removing: $test_dir/expected/sorbet/rbi/hidden-definitions"
    rm -rf "$test_dir/expected/sorbet/rbi/hidden-definitions"
  fi
fi

cp -r "$test_dir/src"/* "$actual"


# ----- Run the test in the sandbox -----

(
  # Only cd because `srb init` needs to be run from the folder with a Gemfile,
  # not because this test driver needs to refer to files with relative paths.
  cd "$actual"

  SRB_YES=1 bundle exec "$srb" init | \
    sed -e 's/with [0-9]* modules and [0-9]* aliases/with X modules and Y aliases/' \
    > "$actual/out.log"
)


# ----- Check out.log -----

if [ -z "$is_partial" ] || [ -f "$test_dir/expected/out.log" ]; then
  if ! diff -u "$test_dir/expected/out.log" "$actual/out.log"; then
    error "├─ expected out.log did not match actual out.log"

    if [ -z "$UPDATE" ]; then
      error "└─ see output above."
      exit 1
    else
      warn "├─ updating expected/out.log"
      mkdir -p "$test_dir/expected"
      cp "$actual/out.log" "$test_dir/expected/out.log"
    fi
  fi
fi

# ----- Check sorbet/ -----

# FIXME: Removing hidden-definitions in actual to hide them from diff output.
rm -rf "$actual/sorbet/rbi/hidden-definitions"

diff_total() {
  if ! diff -ur "$test_dir/expected/sorbet" "$actual/sorbet"; then
    error "├─ expected sorbet/ folder did not match actual sorbet/ folder"

    if [ -z "$UPDATE" ]; then
      error "└─ see output above."
      exit 1
    else
      warn "├─ updating expected/sorbet (total):"
      rm -rf "$test_dir/expected/sorbet"
      cp -r "$actual/sorbet" "$test_dir/expected"
    fi
  fi
}

diff_partial() {
  set +e
  diff -ur "$test_dir/expected/sorbet" "$actual/sorbet" | \
    grep -vF "Only in $actual" \
    > "$actual/partial-diff.log"
  set -e

  # File exists and is non-empty
  if [ -s "$actual/partial-diff.log" ]; then
    cat "$actual/partial-diff.log"
    error "├─ expected sorbet/ folder did not match actual sorbet/ folder"

    if [ -z "$UPDATE" ]; then
      error "└─ see output above."
      exit 1
    else
      warn "├─ updating expected/sorbet (partial):"

      find "$test_dir/expected/sorbet" -print0 | while IFS= read -r -d '' expected_path; do
        path_suffix="${expected_path#$test_dir/expected/sorbet}"
        actual_path="$actual/sorbet$path_suffix"

        # Only ever update existing files, never grow this partial snapshot.
        if [ -d "$expected_path" ]; then
          if ! [ -d "$actual_path" ]; then
            rm -rfv "$expected_path"
          fi
        else
          if [ -f "$actual_path" ]; then
            cp -v "$actual_path" "$expected_path"
          else
            rm -fv "$expected_path"
          fi
        fi
      done
    fi
  fi
}

if [ -z "$is_partial" ]; then
  diff_total
elif [ -d "$test_dir/expected" ]; then
  diff_partial
elif [ -n "$UPDATE" ]; then
  warn "├─ treating empty partial test as total for the sake of updating."
  warn "├─ Feel free to delete files in this snapshot that you don't want."
  diff_total
else
  # It's fine for a partial test to not have an expected dir.
  # It means the test only cares about the exit code of srb init.
  true
fi

rm -rf "$actual"

success "└─ test passed."
