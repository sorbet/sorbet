#!/bin/bash
dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT
set -e
main/sorbet --silence-dev-message -vvv --preallocate-symbol-size 4000 --preallocate-name-size 4000 --cache-dir "$dir" test/cli/cache-reserve-mem/input.rb
main/sorbet --silence-dev-message -vvv --preallocate-symbol-size 4000 --preallocate-name-size 4000 --cache-dir "$dir" test/cli/cache-reserve-mem/input.rb
echo "--preallocate-symbol-size --preallocate-name-size OK"
