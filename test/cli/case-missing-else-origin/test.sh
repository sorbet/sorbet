#!/usr/bin/env bash

for parser in original prism; do
  echo "--- --parser=$parser"
  main/sorbet --silence-dev-message --parser="$parser" test/cli/case-missing-else-origin/case_missing_else_origin.rb 2>&1
done
