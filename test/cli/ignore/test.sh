#!/bin/bash

# Tests both relative and absolute ignore patterns.
main/sorbet --silence-dev-message --ignore foo.rb --ignore /subfolder --dir test/cli/ignore 2>&1
