#!/bin/bash
set -euo pipefail

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

mkdir "$dir/cache"
cat >> "$dir/test.rb" <<EOF
# typed: true
class A
  def self.hi; end
  hi
end
A.how_are_you
EOF

echo ====cold cache====
main/sorbet \
    --silence-dev-message \
    --dsl-plugins test/cli/subprocess-plugin-caching-single-threaded/triggers.yaml \
    --cache-dir "$dir/cache" \
    "$dir/test.rb" 2>&1
echo ====hot cache====
# since we cached the output of the plugin, it should work even though --dsl-plugins is gone.
main/sorbet --silence-dev-message --cache-dir "$dir/cache" "$dir/test.rb" 2>&1
echo ====invalidation====
echo '# comment' >> "$dir/test.rb"
output=$(main/sorbet --silence-dev-message --cache-dir "$dir/cache" "$dir/test.rb" 2>&1 || true)
echo "$output" | grep -o 'Method `how_are_you` does not exist'
echo "$output" | tail -n 1
