#!/bin/bash

main/sorbet \
    --silence-dev-message \
    --stop-after parser \
    --no-error-count \
    -p parse-tree-whitequark \
    test/cli/parse-tree-whitequark/parse-tree-whitequark.rb 2>&1
