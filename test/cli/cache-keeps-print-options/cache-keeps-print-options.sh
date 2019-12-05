#!/bin/bash

CACHE=$(mktemp -d)

main/sorbet --silence-dev-message --censor-for-snapshot-tests -p symbol-table --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
main/sorbet --silence-dev-message --censor-for-snapshot-tests -p symbol-table --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
rm -rf "$CACHE"/*.mdb

main/sorbet --silence-dev-message -p desugar-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
main/sorbet --silence-dev-message -p desugar-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
rm -rf "$CACHE"/*.mdb

main/sorbet --silence-dev-message -p rewrite-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
main/sorbet --silence-dev-message -p rewrite-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
rm -rf "$CACHE"/*.mdb

main/sorbet --silence-dev-message -p index-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
main/sorbet --silence-dev-message -p index-tree --cache-dir="$CACHE" test/cli/cache-keeps-print-options/cache-keeps-print-options.rb 2>/dev/null
