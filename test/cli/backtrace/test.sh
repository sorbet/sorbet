#!/bin/bash
outfile=$(mktemp)
PASS_TEXT="backtrace has function names"
FAIL_TEXT="backtrace does not have function names"
cleanup() {
    rm "$outfile"
}
trap cleanup EXIT
set -e


main/sorbet --silence-dev-message --version &> "$outfile"
if ! grep -q "with debug symbols" "$outfile"
then
 # this is build without debug symbols. the test does not make sense for it. Let it pass
 echo "$PASS_TEXT"
 exit 0;
fi

main/sorbet --silence-dev-message --simulate-crash  &> "$outfile" || true
if grep -q "realmain" "$outfile"
then
    echo "$PASS_TEXT"
else
    echo "$FAIL_TEXT"
    cat "$outfile"
fi
