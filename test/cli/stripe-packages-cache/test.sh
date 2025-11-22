#!/bin/bash
set -euo pipefail

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

mkdir "$dir/cache"
echo $'# typed: strict\nclass Root < PackageSpec; end' > "$dir/__package.rb"
echo "class Root::A; end" > "$dir/a.rb"
echo "class Root::B; end" > "$dir/b.rb"
echo "class Root::C; end" > "$dir/c.rb"

run_sorbet() {
  if ! main/sorbet --silence-dev-message --sorbet-packages --counters --cache-dir "$dir/cache" "$dir"/{__package,a,b,c}.rb 2> "$dir/stderr.txt"; then
    cat "$dir/stderr.txt"
    exit 1
  fi
  grep 'cache.abort\|types.input.files\(.kvstore.miss\|.kvstore.hit\|.kvstore.write\)\? :' "$dir/stderr.txt"
}

echo "====first run (cold cache)===="
run_sorbet
echo "====second run (warm cache)===="
run_sorbet

echo '# comment' >> "$dir/a.rb"

echo "====first after change (one cache miss)===="
run_sorbet
echo "====second after change (warm cache)===="
run_sorbet
