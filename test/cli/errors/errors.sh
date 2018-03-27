#!/bin/bash

main/ruby-typer test/cli/errors/errors.rb 2>&1

CACHE=$(mktemp -d)

main/ruby-typer --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1
main/ruby-typer --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1
