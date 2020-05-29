#!/bin/bash
dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT
set -e
main/sorbet --silence-dev-message -vvv --reserve-symbol-table-capacity 4000 --reserve-name-table-capacity 4000 --cache-dir "$dir" test/cli/cache-reserve-mem/input.rb
main/sorbet --silence-dev-message -vvv --reserve-symbol-table-capacity 4000 --reserve-name-table-capacity 4000 --cache-dir "$dir" test/cli/cache-reserve-mem/input.rb
echo "--reserve-symbol-table-capacity --reserve-name-table-capacity OK"
