#!/bin/bash

CACHE=$(mktemp -d)

main/ruby-typer -p name-table --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
main/ruby-typer -p name-table --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
rm -rf "$CACHE"/*.mdb

main/ruby-typer -p ast --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
main/ruby-typer -p ast --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
rm -rf "$CACHE"/*.mdb

main/ruby-typer -p dsl-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
main/ruby-typer -p dsl-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
