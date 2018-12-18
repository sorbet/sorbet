#!/bin/bash
dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT
set -e
main/sorbet --silence-dev-message -vvv --reserve-mem-kb 128000 --cache-dir "$dir" test/cli/cache-reserve-mem/input.rb
main/sorbet --silence-dev-message -vvv --reserve-mem-kb 128000 --cache-dir "$dir" test/cli/cache-reserve-mem/input.rb
echo "--reserve-mem-kb OK"
