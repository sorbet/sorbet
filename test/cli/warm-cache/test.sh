#!/bin/bash
set -euo pipefail

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

mkdir "$dir/cache"
echo "class A; end" > "$dir/a.rb"
echo "class B; end" > "$dir/b.rb"
echo "class C; end" > "$dir/c.rb"

run_sorbet() {
  if ! main/sorbet --silence-dev-message --counters --cache-dir "$dir/cache" "$dir"/{a,b,c}.rb 2> "$dir/stderr.txt"; then
    cat "$dir/stderr.txt"
    exit 1
  fi
  grep 'types.input.files\(.kvstore.miss\|.kvstore.hit\|.kvstore.write\)\? :' "$dir/stderr.txt"
}

echo "====first run (cold cache)===="
run_sorbet
echo "====second run (warm cache)===="
run_sorbet

echo 'class A2; end' >> "$dir/a.rb"

echo "====first after change (one cache miss)===="
run_sorbet
echo "====second after change (warm cache)===="
run_sorbet

echo '# comment' >> "$dir/a.rb"

echo "====first after comment change (one cache miss)===="
run_sorbet
echo "====second after comment change (warm cache)===="
run_sorbet
