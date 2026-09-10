#!/usr/bin/env bash

root="$PWD"

cd test/cli/test-packages-ignore-test-import || ( echo "Failed to cd!"; exit 1 )

args=(--censor-for-snapshot-tests --silence-dev-message --max-threads=0 --sorbet-packages --experimental-test-packages)

echo '------ monolithic -------------------------------'
"$root/main/sorbet" "${args[@]}" . 2>&1

echo '------ package-directed -------------------------'
"$root/main/sorbet" "${args[@]}" --experimental-package-directed . 2>&1
