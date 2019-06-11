#!/bin/bash

CACHE=$(mktemp -d)

main/sorbet --silence-dev-message -p symbol-table --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
main/sorbet --silence-dev-message -p symbol-table --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
rm -rf "$CACHE"/*.mdb

main/sorbet --silence-dev-message -p ast --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
main/sorbet --silence-dev-message -p ast --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
rm -rf "$CACHE"/*.mdb

main/sorbet --silence-dev-message -p dsl-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
main/sorbet --silence-dev-message -p dsl-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
rm -rf "$CACHE"/*.mdb

main/sorbet --silence-dev-message -p index-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
main/sorbet --silence-dev-message -p index-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
