#!/bin/bash

CACHE=$(mktemp -d)

main/ruby-typer -p name-table --cache-dir="$CACHE" test/cli/cache-keeps-name-table/cache-keeps-name-table.rb 2>&1
main/ruby-typer -p name-table --cache-dir="$CACHE" test/cli/cache-keeps-name-table/cache-keeps-name-table.rb 2>&1
