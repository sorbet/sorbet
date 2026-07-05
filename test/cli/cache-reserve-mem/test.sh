#!/bin/bash
dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT
set -e
main/sorbet --silence-dev-message -vvv --reserve-class-table-capacity 4000 --reserve-method-table-capacity 4000 --reserve-field-table-capacity 4000 --reserve-type-argument-table-capacity 4000 --reserve-type-member-table-capacity 4000 --reserve-utf8-name-table-capacity 4000 --reserve-constant-name-table-capacity 4000 --reserve-unique-name-table-capacity 4000 --cache-dir "$dir" test/cli/cache-reserve-mem/input.rb
main/sorbet --silence-dev-message -vvv --reserve-class-table-capacity 4000 --reserve-method-table-capacity 4000 --reserve-field-table-capacity 4000 --reserve-type-argument-table-capacity 4000 --reserve-type-member-table-capacity 4000 --reserve-utf8-name-table-capacity 4000 --reserve-constant-name-table-capacity 4000 --reserve-unique-name-table-capacity 4000 --cache-dir "$dir" test/cli/cache-reserve-mem/input.rb
echo '--reserve-*-table-capacity OK'
