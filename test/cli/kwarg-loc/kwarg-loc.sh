#!/bin/bash

# Tests both relative and absolute ignore patterns.
main/sorbet --silence-dev-message --dir test/cli/kwarg-loc 2>&1
