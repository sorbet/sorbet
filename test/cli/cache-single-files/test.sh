#!/usr/bin/env bash

dir="$(mktemp -d)"
test_dir="$(dirname "${BASH_SOURCE[0]}")"

trap 'rm -rf "$dir"' EXIT

mkdir "$dir/cache"

cp "$test_dir/test.rb" "$dir"

run_sorbet() {
  if ! main/sorbet --silence-dev-message --counters --cache-dir "$dir/cache" "$dir/test.rb" 2> "$dir/stderr.txt"; then
    cat "$dir/stderr.txt"
    exit 1
  fi

  grep 'types\.input\.files\.kvstore\.\(miss\|hit\|write\)' "$dir/stderr.txt"
}

echo "====first run (cold cache)===="
run_sorbet
echo "====second run (warm cache)===="
run_sorbet

echo "====third run (cache miss due to file modification)===="
echo "# trailing comment" >> "$dir/test.rb"
run_sorbet

echo "====fourth run (cache miss after restoring original file)===="
cp "$test_dir/test.rb" "$dir"
run_sorbet
