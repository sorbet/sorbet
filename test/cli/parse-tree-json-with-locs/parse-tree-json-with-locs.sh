#!/bin/bash

main/sorbet \
    --silence-dev-message \
    --stop-after parser \
    --no-error-count \
    -p parse-tree-json-with-locs \
    test/cli/parse-tree-json-with-locs/parse-tree-json-with-locs1.rb \
    test/cli/parse-tree-json-with-locs/parse-tree-json-with-locs2.rb \
    test/cli/parse-tree-json-with-locs/parse-tree-json-with-locs3.rb 2>&1
