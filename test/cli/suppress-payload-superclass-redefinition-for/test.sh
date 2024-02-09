#!/usr/bin/env bash

set -euo pipefail

cwd="$(pwd)"
cd test/cli/suppress-payload-superclass-redefinition-for

echo "-- complex.rbi --"

2>&1 "$cwd/main/sorbet" --silence-dev-message \
  --suppress-payload-superclass-redefinition-for=Complex complex.rbi

echo "-- complex.rbi --"
2>&1 "$cwd/main/sorbet" --silence-dev-message \
  --suppress-payload-superclass-redefinition-for=IRB::RelineInputMethod irb.rbi

echo "-- expect constant resolution error --"
if 2>&1 "$cwd/main/sorbet" --silence-dev-message -e '' \
  --suppress-payload-superclass-redefinition-for=DoesNotExist; then
  echo "Expected to fail!"
  exit 1
fi


echo "-- double.rb --"
2>&1 "$cwd/main/sorbet" --silence-dev-message \
  --suppress-payload-superclass-redefinition-for=Complex \
  --suppress-payload-superclass-redefinition-for=IRB::RelineInputMethod \
  double.rb

