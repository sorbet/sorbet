#!/bin/bash

main/sorbet --silence-dev-message test/cli/errors/errors.rb 2>&1

CACHE=$(mktemp -d)

main/sorbet --silence-dev-message --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1
main/sorbet --silence-dev-message --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1
