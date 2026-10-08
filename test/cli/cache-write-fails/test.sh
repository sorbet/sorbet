#!/usr/bin/env bash
set -euo pipefail

dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT

# Failing to write to the cache (e.g. because it has reached --max-cache-size-bytes) should not
# crash Sorbet: it should warn, and then report errors as though there were no cache.
run_sorbet() {
  local code=0
  main/sorbet --silence-dev-message --cache-dir="$dir/cache" "$@" > "$dir/out.txt" 2>&1 || code=$?
  sed -e "s#$dir/##g" "$dir/out.txt"
  echo "exit code: $code"
}

echo '# typed: true' > "$dir/error.rb"
echo 'T.let(1, String)' >> "$dir/error.rb"

echo "==== global state too large for the cache ===="
mkdir "$dir/names"
{
  echo '# typed: true'
  for i in $(seq 1 2000); do
    echo "class A$i; def foo$i(x); x + $i; end; end"
  done
} > "$dir/names/names.rb"
run_sorbet --max-cache-size-bytes=65536 "$dir/error.rb" "$dir/names"
rm -rf "$dir/cache"

echo "==== cache becomes full while writing trees ===="
mkdir "$dir/trees"
for f in $(seq 1 400); do
  {
    echo '# typed: false'
    for i in $(seq 1 50); do
      echo "foo(bar, baz, 1, 2, 3)"
    done
  } > "$dir/trees/f$f.rb"
done
# The first run fits in the cache...
run_sorbet --max-cache-size-bytes=262144 "$dir/error.rb" "$dir/trees/f1.rb"
# ... but caching all the other files does not.
run_sorbet --max-cache-size-bytes=262144 "$dir/error.rb" "$dir/trees"
# The cache is still usable afterwards.
run_sorbet --max-cache-size-bytes=262144 "$dir/error.rb" "$dir/trees/f1.rb"
