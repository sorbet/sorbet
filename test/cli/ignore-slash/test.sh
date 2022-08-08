#!/bin/bash

# Tests both relative and absolute ignore patterns.
main/sorbet --censor-for-snapshot-tests --silence-dev-message --ignore ignore_me/ --dir test/cli/ignore-slash 2>&1
