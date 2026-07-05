#!/bin/bash

main/sorbet \
    --silence-dev-message \
    --stop-after parser \
    --no-error-count \
    -p parse-tree-json \
    test/cli/parse-tree-json/parse-tree-json.rb 2>&1
