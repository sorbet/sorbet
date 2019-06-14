#!/bin/bash

# Tests both relative and absolute ignore patterns.
main/sorbet --silence-dev-message --ignore foo/ -d test/cli/ignore-slash 2>&1
