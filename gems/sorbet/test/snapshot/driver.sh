#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/../.." || exit 1

# shellcheck disable=SC1091
source "test/snapshot/logging.sh"

# ----- Option parsing -----

usage() {
  echo
  echo "Usage:"
  echo "  $0 [options]"
  echo
  echo "Options:"
  echo "  --verbose    Be more verbose (more than just test summary)."
  echo "  --update     If a test fails, overwrite the expected with the actual"
}

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

# ----- Discover and run all the tests -----

info "Running suite: $0 $VERBOSE $UPDATE"

if [ -z "$VERBOSE" ]; then
  info "(re-run with --verbose for more information)"
else
  info "(--verbose mode)"
fi

passing_tests=()
failing_tests=()

for test_dir in test/snapshot/{partial,total}/*; do
  if test/snapshot/test_one.sh "$test_dir" $VERBOSE $UPDATE; then
    passing_tests+=("test/snapshot/test_one.sh $test_dir $VERBOSE $UPDATE")
  else
    failing_tests+=("test/snapshot/test_one.sh $test_dir $VERBOSE $UPDATE")
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
