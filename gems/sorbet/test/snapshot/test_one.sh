#!/usr/bin/env bash

set -euo pipefail
shopt -s dotglob

pushd "$(dirname "${BASH_SOURCE[0]}")/../.." &> /dev/null
root_dir="$(realpath "$PWD")"
popd &> /dev/null

# shellcheck disable=SC1090
source "$root_dir/test/snapshot/logging.sh"

# ----- Helper functions -----

wrap_verbose() {
  out_log="$(mktemp)"
  # shellcheck disable=SC2064
  trap "rm -f '$out_log'" EXIT

  if ! "$@" > "$out_log" 2>&1; then
    if [ "$VERBOSE" = "" ]; then
      error "└─ '$*' failed. Re-run with --verbose for more."
    else
      cat "$out_log"
      error "└─ '$*' failed. See output above."
    fi
    exit 1
  fi
  if [ "$VERBOSE" != "" ]; then
    cat "$out_log"
  fi
}

# ----- Option parsing -----

usage() {
  echo
  echo "Usage:"
  echo "  $0 <test_dir> [options]"
  echo
  echo "Arguments:"
  echo "  <test_dir>   path to the snapshot to test"
  echo
  echo "Options:"
  echo "  --verbose    Be more verbose than just whether it errored"
  echo "  --update     If there is a failure, update the expected file(s) and keep going"
  echo "  --record     Treat partial tests as total tests for the purpose of creating"
  echo "               a new test. Requires --update"
  echo "  --debug      Don't redirect srb output so that it can be debugged"
}

if [[ $# -lt 1 ]]; then
  error "Missing test."
  usage
  exit 1
elif ! [ -d "$1" ]; then
  error "test_dir doesn't exist: $1"
  exit 1
else
  # $1 might be either absolute or relative. For example, driver.sh always uses
  # absolute paths, but we always print relative paths to re-run a single test.
  relative_test_dir="$(realpath --relative-to="$PWD" "$1")"
  test_dir="$(realpath "$1")"
fi
shift

if [[ "$test_dir" =~ $root_dir/test/snapshot/partial* ]]; then
  is_partial=1
elif [[ "$test_dir" =~ $root_dir/test/snapshot/total* ]]; then
  is_partial=
else
  error "Could not determine whether partial or total."
  error "Expected: $root_dir/test/snapshot/(total|partial)/*"
  error "Got:      $test_dir"
  usage
  exit 1
fi

VERBOSE=
UPDATE=
RECORD=
DEBUG=
FLAGS=""
while [[ $# -gt 0 ]]; do
  case $1 in
    --verbose)
      VERBOSE="--verbose"
      FLAGS="$FLAGS $VERBOSE"
      shift
      ;;
    --update)
      UPDATE="--update"
      FLAGS="$FLAGS $UPDATE"
      shift
      ;;
    --record)
      RECORD="--record"
      FLAGS="$FLAGS $RECORD"
      shift
      ;;
    --debug)
      DEBUG="--debug"
      FLAGS="$FLAGS $DEBUG"
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

if ! [ "$RECORD" = "" ] && [ "$UPDATE" = "" ]; then
  error "--record requires --update"
  exit 1
fi


# ----- Stage the test sandbox directory -----

relative_test_exe="$(realpath --relative-to="$PWD" "$0")"
info "Running test:  $relative_test_exe $relative_test_dir $FLAGS"

actual="$(mktemp -d)"

if [ "$VERBOSE" != "" ]; then
  info "├─ PWD:       $PWD"
  info "├─ test_dir:  $test_dir"
  info "├─ actual:    $actual"
fi

srb="$root_dir/bin/srb"

if [ "${SRB_SORBET_EXE:-}" = "" ]; then
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

if ! [ -d "$test_dir/src" ]; then
  error "└─ each test must have a src/ dirctory: $test_dir/src"
  exit 1
fi

if ! [ -f "$test_dir/src/Gemfile" ]; then
  error "├─ each test must have src/Gemfile: $test_dir/src/Gemfile"
  if [ "$UPDATE" = "" ]; then
    attn "└─ re-run with --update to create it."
    exit 1
  else
    attn "├─ creating empty Gemfile"
    touch "$test_dir/src/Gemfile"
  fi
fi

if ! [ -f "$test_dir/src/Gemfile.lock" ]; then
  error "├─ each test must have src/Gemfile.lock: $test_dir/src/Gemfile.lock"
  if [ "$UPDATE" = "" ]; then
    attn "└─ re-run with --update to create it."
    exit 1
  else
    attn "├─ running 'bundle install' to create it"
    (
      cd "$test_dir/src"
      bundle install
    )
  fi
fi

# TODO: make these files recordable
if [ -d "$test_dir/expected/sorbet/rbi/hidden-definitions" ]; then
  error "├─ hidden-definitions are not currently testable."

  if [ "$UPDATE" = "" ]; then
    attn "└─ please remove: $test_dir/expected/sorbet/rbi/hidden-definitions"
    exit 1
  else
    attn "├─ removing: $test_dir/expected/sorbet/rbi/hidden-definitions"
    rm -rf "$test_dir/expected/sorbet/rbi/hidden-definitions"
  fi
fi

cp -r "$test_dir/src" "$actual"
if [ -d "$test_dir/gems" ]; then
  cp -r "$test_dir/gems" "$actual"
fi


# ----- Run the test in the sandbox -----

SRB_SORBET_TYPED_REVISION="$(<"$root_dir/test/snapshot/sorbet-typed.rev")"
export SRB_SORBET_TYPED_REVISION

(
  # Only cd because `srb init` needs to be run from the folder with a Gemfile,
  # not because this test driver needs to refer to files with relative paths.
  cd "$actual/src"

  # Install what's installed in the Gemfile.lock (ignoring Gemfile)
  wrap_verbose bundle install

  # Make sure what's in the Gemfile matches what's in the Gemfile.lock
  # (running this in the sandbox, because this will update the Gemfile.lock)
  wrap_verbose bundle check
  if ! diff -u "$test_dir/src/Gemfile.lock" "$actual/src/Gemfile.lock"; then
    error "├─ expected Gemfile.lock did not match actual Gemfile.lock"

    if [ "$UPDATE" = "" ]; then
      error "└─ see output above."
      if [ "$DEBUG" = "" ]; then
        # Debugging usually requires changing the Gemfile to add pry. Printing
        # but not exiting here lets us skip updating for the sake of debugging.
        exit 1
      fi
    else
      attn "├─ updating Gemfile.lock"
      cp "$actual/src/Gemfile.lock" "$test_dir/src/Gemfile.lock"
    fi
  fi

  if [ "$DEBUG" != "" ]; then
    # Don't redirect anything, so that binding.pry and friends work
    SRB_YES=1 bundle exec "$srb" init
    error "Exiting since debug mode doesn't run tests"
    exit 1
  else
    # Uses /dev/null for stdin so any binding.pry would exit immediately
    # (otherwise, pry will be waiting for input, but it's impossible to tell
    # because the pry output is hiding in the *.log files)
    #
    # note: redirects stderr before the pipe
    if ! SRB_YES=1 bundle exec "$srb" init < /dev/null 2> "$actual/src/err.log" \
        | sed -e 's/with [0-9]* modules and [0-9]* aliases/with X modules and Y aliases/' \
        | sed -e 's,/var/folders/[^ ]*/\([^/]*\),<tmp>/\1,g'
        > "$actual/src/out.log"; then
      error "├─ srb init failed."
      if [ "$VERBOSE" = "" ]; then
        error "├─ stdout: $actual/src/out.log"
        error "├─ stderr: $actual/src/err.log"
        error "└─ (or re-run with --verbose)"
      else
        error "├─ stdout ($actual/src/out.log):"
        cat "$actual/src/out.log"
        error "├─ stderr ($actual/src/err.log):"
        cat "$actual/src/err.log"
        error "└─ (end stderr)"
      fi
      exit 1
    fi
  fi
)

# ----- Check out.log -----

if [ "$is_partial" = "" ] || [ -f "$test_dir/expected/out.log" ]; then
  if ! diff -u "$test_dir/expected/out.log" "$actual/src/out.log"; then
    error "├─ expected out.log did not match actual out.log"

    if [ "$UPDATE" = "" ]; then
      error "└─ see output above."
      exit 1
    else
      attn "├─ updating expected/out.log"
      mkdir -p "$test_dir/expected"
      cp "$actual/src/out.log" "$test_dir/expected/out.log"
    fi
  fi
fi

# ----- Check err.log -----

if [ "$is_partial" = "" ] || [ -f "$test_dir/expected/err.log" ]; then
  if ! diff -u "$test_dir/expected/err.log" "$actual/src/err.log"; then
    error "├─ expected err.log did not match actual err.log"

    if [ "$UPDATE" = "" ]; then
      error "└─ see output above."
      exit 1
    else
      attn "├─ updating expected/err.log"
      cp "$actual/src/err.log" "$test_dir/expected/err.log"
    fi
  fi
fi

# ----- Check sorbet/ -----

# FIXME: Removing hidden-definitions in actual to hide them from diff output.
rm -rf "$actual/src/sorbet/rbi/hidden-definitions"

diff_total() {
  if ! diff -ur "$test_dir/expected/sorbet" "$actual/src/sorbet"; then
    error "├─ expected sorbet/ folder did not match actual sorbet/ folder"

    if [ "$UPDATE" = "" ]; then
      error "└─ see output above. Run with --update to fix."
      exit 1
    else
      attn "├─ updating expected/sorbet (total):"
      rm -rf "$test_dir/expected/sorbet"
      mkdir -p "$test_dir/expected"
      cp -r "$actual/src/sorbet" "$test_dir/expected"
    fi
  fi
}

diff_partial() {
  set +e
  diff -ur "$test_dir/expected/sorbet" "$actual/src/sorbet" | \
    grep -vF "Only in $actual" \
    > "$actual/src/partial-diff.log"
  set -e

  # File exists and is non-empty
  if [ -s "$actual/src/partial-diff.log" ]; then
    cat "$actual/src/partial-diff.log"
    error "├─ expected sorbet/ folder did not match actual sorbet/ folder"

    if [ "$UPDATE" = "" ]; then
      error "└─ see output above."
      exit 1
    else
      attn "├─ updating expected/sorbet (partial):"

      find "$test_dir/expected/sorbet" -print0 | while IFS= read -r -d '' expected_path; do
        path_suffix="${expected_path#$test_dir/expected/sorbet}"
        actual_path="$actual/src/sorbet$path_suffix"

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

if [ "$is_partial" = "" ]; then
  diff_total
elif [ -d "$test_dir/expected/sorbet" ]; then
  diff_partial
elif [ "$UPDATE" != "" ]; then
  if [ "$RECORD" = "" ]; then
    info "├─ Not recording sorbet/ folder for empty partial test (add --record if you wanted this)."
  else
    info "├─ Treating empty partial test as total for the sake of recording."
    diff_total
  fi
else
  # It's fine for a partial test to not have an expected dir.
  # It means the test only cares about the exit code of srb init.
  true
fi

rm -rf "$actual"

success "└─ test passed."
