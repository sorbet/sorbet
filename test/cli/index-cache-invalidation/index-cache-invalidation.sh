#!/bin/bash
set -euo pipefail

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

mkdir "$dir/cache"
echo "class A; end" >> "$dir/a.rb"
touch "$dir"{b,c,d}
echo ====first run====
main/sorbet --silence-dev-message --no-stdlib --cache-dir "$dir/" "$dir/a.rb" "$dir"{b,c,d} 2>&1
echo '# comment' >> "$dir/a.rb"
echo ====after change====
main/sorbet --silence-dev-message --no-stdlib --cache-dir "$dir/" "$dir/a.rb" "$dir"{b,c,d} 2>&1
