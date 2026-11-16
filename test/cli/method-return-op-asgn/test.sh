#!/usr/bin/env bash

for parser in original prism; do
  echo "--- --parser=$parser"
  main/sorbet --silence-dev-message --parser="$parser" test/cli/method-return-op-asgn/method_return_op_asgn.rb 2>&1
done
