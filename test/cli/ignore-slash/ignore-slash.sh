#!/bin/bash

# Tests both relative and absolute ignore patterns.
main/sorbet --silence-dev-message --ignore foo/ test/cli/ignore-slash 2>&1
