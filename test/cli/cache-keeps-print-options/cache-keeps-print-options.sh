#!/bin/bash

CACHE=$(mktemp -d)

main/sorbet --silence-dev-message -p symbol-table --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
main/sorbet --silence-dev-message -p symbol-table --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
rm -rf "$CACHE"/*.mdb

main/sorbet --silence-dev-message -p ast --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
main/sorbet --silence-dev-message -p ast --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
rm -rf "$CACHE"/*.mdb

main/sorbet --silence-dev-message -p dsl-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
main/sorbet --silence-dev-message -p dsl-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>&1
