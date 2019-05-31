#!/usr/bin/env bash

set -euo pipefail

pushd "$(dirname "${BASH_SOURCE[0]}")/../.." &> /dev/null
root_dir="$(realpath "$PWD")"
popd &> /dev/null

# shellcheck disable=SC1090
source "$root_dir/test/snapshot/logging.sh"

# ----- Option parsing -----

usage() {
  echo
  echo "Usage:"
  echo "  $0 [options]"
  echo
  echo "Options:"
  echo "  --verbose    Be more verbose (more than just test summary)."
  echo "  --update     If a test fails, overwrite the expected with the actual"
  echo "  --record     Treat partial tests as total tests for the purpose of creating"
  echo "               new tests. Requires --update"
}

VERBOSE=
UPDATE=
RECORD=
FLAGS=()
while [[ $# -gt 0 ]]; do
  case $1 in
    --verbose)
      VERBOSE="--verbose"
      FLAGS+=("$VERBOSE")
      shift
      ;;
    --update)
      UPDATE="--update"
      FLAGS+=("$UPDATE")
      shift
      ;;
    --record)
      RECORD="--record"
      FLAGS+=("$RECORD")
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

# ----- Discover and run all the tests -----

info "Running suite: $0 ${FLAGS[*]}"

if [ -z "$VERBOSE" ]; then
  info "(re-run with --verbose for more information)"
else
  info "(--verbose mode)"
fi

passing_tests=()
failing_tests=()

for test_dir in "$root_dir/test/snapshot"/{partial,total}/*; do
  test_exe="$root_dir/test/snapshot/test_one.sh"

  relative_test_exe="$(realpath --relative-to="$PWD" "$test_exe")"
  relative_test_dir="$(realpath --relative-to="$PWD" "$test_dir")"

  if "$test_exe" "$test_dir" "${FLAGS[@]}"; then
    passing_tests+=("$relative_test_exe $relative_test_dir ${FLAGS[*]}")
  else
    failing_tests+=("$relative_test_exe $relative_test_dir ${FLAGS[*]}")
  fi
done


if [ "${#passing_tests[@]}" -ne 0 ]; then
  echo
  echo "───── Passing tests ─────"
  for passing_test in "${passing_tests[@]}"; do
    success "$passing_test"
  done
fi

if [ "${#failing_tests[@]}" -ne 0 ]; then
  echo
  echo "───── Failing tests ─────"

  for failing_test in "${failing_tests[@]}"; do
    error "$failing_test"
  done

  if [ -z "$UPDATE" ]; then
    echo
    info "If you have reason to believe these differences are ok to keep, run"
    info "    $0 $* --update"
    info "or append --update to any of the commands above to update a single test."
    echo
  fi

  exit 1
fi
