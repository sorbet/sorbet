#!/bin/bash
set -euo pipefail

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

touch "$dir/"{a,b,c,d}
mkdir "$dir/cache"
cat >> "$dir/test.rb" <<EOF
# typed: true
class A
  def self.hi; end
  hi
end
A.how_are_you
EOF

# --max-threads 1 to work around Github #1076. Run through the paralel code path

echo ====cold cache====
main/sorbet --silence-dev-message --max-threads 1 --dsl-plugins test/cli/subprocess-plugin-caching/triggers.yaml --cache-dir "$dir/cache" "$dir/test.rb" "$dir/"{a,b,c,d} 2>&1
echo ====hot cache====
# since we cached the output of the plugin, it should work even though --dsl-plugins is gone.
main/sorbet --silence-dev-message --max-threads 1 --cache-dir "$dir/cache" "$dir/test.rb" "$dir/"{a,b,c,d} 2>&1
echo ====invalidation====
echo '# comment' >> "$dir/test.rb"
output=$(main/sorbet --silence-dev-message --max-threads 1 --cache-dir "$dir/cache" "$dir/test.rb" "$dir/"{a,b,c,d} 2>&1 || true)
echo "$output" | grep -o 'Method `how_are_you` does not exist'
echo "$output" | tail -n 1
