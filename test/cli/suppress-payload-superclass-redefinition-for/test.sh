#!/usr/bin/env bash

set -euo pipefail

cd test/cli/suppress-payload-superclass-redefinition-for

echo "-- complex.rbi --"

2>&1 main/sorbet --silence-dev-message \
  --suppress-payload-superclass-redefinition-for=Complex complex.rbi

echo "-- complex.rbi --"
2>&1 main/sorbet --silence-dev-message \
  --suppress-payload-superclass-redefinition-for=IRB::RelineInputMethod irb.rbi

echo "-- expect constant resolution error --"
if ! 2>&1 main/sorbet --silence-dev-message -e '' \
  --suppress-payload-superclass-redefinition-for=DoesNotExist; then
  echo "Expected to fail!"
  exit 1
fi

