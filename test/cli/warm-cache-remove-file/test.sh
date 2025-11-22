#!/bin/bash
set -euo pipefail
exec 2>&1

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

mkdir "$dir/cache"

mkdir "$dir/a"
echo "class A < PackageSpec; end # typed: strict" > "$dir/a/__package.rb"
for i in {01..10}; do
  echo "class A::A$i; end" > "$dir/a/a$i.rb"
done
mkdir "$dir/b"
echo "class B < PackageSpec; end # typed: strict" > "$dir/b/__package.rb"
for i in {01..10}; do
  echo "class B::B$i; end" > "$dir/b/b$i.rb"
done

run_sorbet() {
  if ! main/sorbet --silence-dev-message --counters --cache-dir "$dir/cache" --sorbet-packages "$@" 2> "$dir/stderr.txt"; then
    cat "$dir/stderr.txt"
    exit 1
  fi
  grep 'cache.abort\|types.input.files\(.kvstore.miss\|.kvstore.hit\|.kvstore.write\)\? :' "$dir/stderr.txt"
}

echo "====first run (all files)===="
run_sorbet "$dir"/{a,b}/
echo "====second run (only one file)===="
run_sorbet "$dir"/b/__package.rb "$dir"/b/b10.rb
