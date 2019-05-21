#!/bin/bash
main/sorbet --silence-dev-message test/cli/print_some/*.rb -p cfg \
  --print-file test/cli/print_some/b.rb --print-file test/cli/print_some/c.rb
