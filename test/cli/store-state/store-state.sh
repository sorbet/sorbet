#!/bin/bash
set -e
main/sorbet --silence-dev-message -e 'class Foo; end' --store-state state
if [ ! -f state ]; then
  echo "state file wasn't created"
  exit 1
fi
