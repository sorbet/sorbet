#!/bin/bash

main/sorbet test/cli/errors/errors.rb 2>&1

CACHE=$(mktemp -d)

main/sorbet --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1
main/sorbet --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1
