#!/bin/bash
set -u

out=$(mktemp)
trap 'rm -f $out' EXIT

main/sorbet --silence-dev-message --stripe-mode \
  test/cli/conflicting-definition-multithread/*.rb >"$out" 2>&1

grep "^test/cli/conflicting-definition-multithread/a.* has behavior defined in multiple files https://srb.help/4019$" "$out" |
  sort

echo
echo "== Previous definitions =="
grep ": Previous definition" "$out" | sort
