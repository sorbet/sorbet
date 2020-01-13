#!/bin/bash
dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT
set -e

mkdir "$dir/cache"
cat >> "$dir/test.rb" <<EOF
puts("hi".)
EOF
main/sorbet --silence-dev-message --cache-dir "$dir/cache" "$dir/test.rb" -q 2>&1
echo "should not cache erroring trees"
main/sorbet --silence-dev-message --cache-dir "$dir/cache" "$dir/test.rb" 2>&1 | grep -v "test.rb" # strip file name as it has tmp stuff in it
