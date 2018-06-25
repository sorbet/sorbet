#!/bin/bash
dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT
set -e

mkdir "$dir/cache"
cat >> "$dir/test.rb" <<EOF
puts("hi")
EOF
main/sorbet --cache-dir "$dir/cache" "$dir/test.rb" 2>&1
echo "# typed: true" >> "$dir/test.rb"
main/sorbet --cache-dir "$dir/cache" "$dir/test.rb" 2>&1
