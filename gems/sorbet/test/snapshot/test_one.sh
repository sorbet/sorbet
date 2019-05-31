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
    if [ -z "$VERBOSE" ]; then
      error "└─ '$*' failed. Re-run with --verbose for more."
    else
      cat "$out_log"
      error "└─ '$*' failed. See output above."
    fi
    exit 1
  fi
  if [ -n "$VERBOSE" ]; then
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
  echo "  --installed  Test using the version of 'srb' that's currently installed,"
  echo "               rather than using the development version."
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

INSTALLED=
VERBOSE=
UPDATE=
RECORD=
DEBUG=
FLAGS=""
while [[ $# -gt 0 ]]; do
  case $1 in
    --installed)
      VERBOSE="--installed"
      FLAGS+=("$INSTALLED")
      shift
      ;;
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

if ! [ -z "$RECORD" ] && [ -z "$UPDATE" ]; then
  error "--record requires --update"
  exit 1
fi


# ----- Stage the test sandbox directory -----

relative_test_exe="$(realpath --relative-to="$PWD" "$0")"
info "Running test:  $relative_test_exe $relative_test_dir $FLAGS"

actual="$(mktemp -d)"

if [ -z "$INSTALLED" ]; then
  srb="$root_dir/bin/srb"
else
  srb="srb"
fi

if [ -n "$VERBOSE" ]; then
  info "├─ PWD:       $PWD"
  info "├─ test_dir:  $test_dir"
  info "├─ actual:    $actual"
  info "├─ srb:       $srb"
fi

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

if ! [ -d "$test_dir/src" ]; then
  error "└─ each test must have a src/ dirctory: $test_dir/src"
  exit 1
fi

if ! [ -f "$test_dir/src/Gemfile" ]; then
  error "├─ each test must have src/Gemfile: $test_dir/src/Gemfile"
  if [ -z "$UPDATE" ]; then
    warn "└─ re-run with --update to create it."
    exit 1
  else
    warn "└─ creating empty Gemfile"
    touch "$test_dir/src/Gemfile"
  fi
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

SRB_SORBET_TYPED_REVISION="$(<"$root_dir/test/snapshot/sorbet-typed.rev")"
export SRB_SORBET_TYPED_REVISION

(
  # Only cd because `srb init` needs to be run from the folder with a Gemfile,
  # not because this test driver needs to refer to files with relative paths.
  cd "$actual"

  # Install what's installed in the Gemfile.lock (ignoring Gemfile)
  wrap_verbose bundle install

  # Make sure what's in the Gemfile matches what's in the Gemfile.lock
  # (running this in the sandbox, because this will update the Gemfile.lock)
  wrap_verbose bundle check
  if ! diff -u "$test_dir/src/Gemfile.lock" "$actual/Gemfile.lock"; then
    error "├─ expected Gemfile.lock did not match actual Gemfile.lock"

    if [ -z "$UPDATE" ]; then
      error "└─ see output above."
      if [ -z "$DEBUG" ]; then
        # Debugging usually requires changing the Gemfile to add pry. Printing
        # but not exiting here lets us skip updating for the sake of debugging.
        exit 1
      fi
    else
      warn "└─ updating Gemfile.lock"
      cp "$actual/Gemfile.lock" "$test_dir/src/Gemfile.lock"
    fi
  fi

  if ! [ -z "$DEBUG" ]; then
    # Don't redirect anything, so that binding.pry and friends work
    bundle exec "$srb" init
    exit
  else
    # Uses /dev/null for stdin so any binding.pry would exit immediately
    # (otherwise, pry will be waiting for input, but it's impossible to tell
    # because the pry output is hiding in the *.log files)
    #
    # note: redirects stderr before the pipe
    if ! SRB_YES=1 bundle exec "$srb" init < /dev/null 2> "$actual/err.log" | \
        sed -e 's/with [0-9]* modules and [0-9]* aliases/with X modules and Y aliases/' \
        > "$actual/out.log"; then
      error "├─ srb init failed."
      if [ -z "$VERBOSE" ]; then
        error "├─ stdout: $actual/out.log"
        error "├─ stderr: $actual/err.log"
        error "└─ (or re-run with --verbose)"
      else
        error "├─ stdout ($actual/out.log):"
        cat "$actual/out.log"
        error "├─ stderr ($actual/err.log):"
        cat "$actual/err.log"
        error "└─ (end stderr)"
      fi
      exit 1
    fi
  fi
)

# ----- Check out.log -----

if [ -z "$is_partial" ] || [ -f "$test_dir/expected/out.log" ]; then
  if ! diff -u "$test_dir/expected/out.log" "$actual/out.log"; then
    error "├─ expected out.log did not match actual out.log"

    if [ -z "$UPDATE" ]; then
      error "└─ see output above."
      exit 1
    else
      warn "└─ updating expected/out.log"
      mkdir -p "$test_dir/expected"
      cp "$actual/out.log" "$test_dir/expected/out.log"
    fi
  fi
fi

# ----- Check err.log -----

if [ -z "$is_partial" ] || [ -f "$test_dir/expected/err.log" ]; then
  if ! diff -u "$test_dir/expected/err.log" "$actual/err.log"; then
    error "├─ expected err.log did not match actual err.log"

    if [ -z "$UPDATE" ]; then
      error "└─ see output above."
      exit 1
    else
      warn "└─ updating expected/err.log"
      cp "$actual/err.log" "$test_dir/expected/err.log"
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
      error "└─ see output above. Run with --update to fix."
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
elif [ -d "$test_dir/expected/sorbet" ]; then
  diff_partial
elif [ -n "$UPDATE" ]; then
  if [ -z "$RECORD" ]; then
    warn "├─ Not recording sorbet/ folder for empty partial test."
    info "├─ Re-run with --record to record."
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
