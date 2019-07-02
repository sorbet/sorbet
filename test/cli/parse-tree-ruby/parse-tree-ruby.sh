#!/bin/bash

main/sorbet \
    --silence-dev-message \
    --stop-after parser \
    --no-error-count \
    -p parse-tree-ruby \
    test/cli/parse-tree-ruby/parse-tree-ruby.rb 2>&1
