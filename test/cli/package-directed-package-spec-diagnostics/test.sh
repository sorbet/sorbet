#!/usr/bin/env bash

cd test/cli/package-directed-package-spec-diagnostics || exit 1

root=../../..
args=(
  --censor-for-snapshot-tests
  --silence-dev-message
  --sorbet-packages
  --packager-layers=app
  --max-threads=0
  .
)

echo '------ monolithic ----------------------'
"$root/main/sorbet" "${args[@]}" 2>&1 || true

echo '------ package-directed ----------------'
"$root/main/sorbet" "${args[@]}" --experimental-package-directed 2>&1 || true
