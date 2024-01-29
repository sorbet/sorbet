#!/bin/bash
set -euo pipefail

trap 'rm -rf nope2 nope3 nope4' EXIT

main/sorbet --silence-dev-message --cache-dir=nope2 -e '0' 2>&1

touch nope3
if main/sorbet --silence-dev-message --cache-dir=nope3/subdir -e 0 2>&1; then
  2>&1 echo 'Expected to fail!'
  exit 1
fi

mkdir nope4
chmod -w nope4
if main/sorbet --silence-dev-message --cache-dir=nope4/subdir -e 0 2>&1; then
  2>&1 echo 'Expected to fail!'
  exit 1
fi
