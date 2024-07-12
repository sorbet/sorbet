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

echo "====first run (cold cache)===="
# Explicitly trying to put stdout into a file and stderr to stdout.
# Not trying to send stdout and stderr to the same place
# shellcheck disable=SC2069
main/sorbet --silence-dev-message --cache-dir "$dir/cache" "$dir"/{a,b,c}.rb 2>&1 #"$dir/stderr.txt"
# grep 'No errors! Great job.' "$dir/stderr.txt"
# grep 'types.input' "$dir/stderr.txt"

echo "====second run (warm cache)===="
# Explicitly trying to put stdout into a file and stderr to stdout.
# Not trying to send stdout and stderr to the same place
# shellcheck disable=SC2069
main/sorbet --silence-dev-message --cache-dir "$dir/cache" "$dir"/{a,b,c}.rb 2>&1 #"$dir/stderr.txt"
# grep 'No errors! Great job.' "$dir/stderr.txt"
# grep 'types.input' "$dir/stderr.txt"

echo '# comment' >> "$dir/a.rb"

echo "====first after change (one cache miss)===="
# Explicitly trying to put stdout into a file and stderr to stdout.
# Not trying to send stdout and stderr to the same place
# shellcheck disable=SC2069
main/sorbet --silence-dev-message --cache-dir "$dir/cache" "$dir"/{a,b,c}.rb 2>&1 #"$dir/stderr.txt"
# grep 'No errors! Great job.' "$dir/stderr.txt"
# grep 'types.input' "$dir/stderr.txt"

echo "====second after change (warm cache)===="
# Explicitly trying to put stdout into a file and stderr to stdout.
# Not trying to send stdout and stderr to the same place
# shellcheck disable=SC2069
main/sorbet --silence-dev-message --cache-dir "$dir/cache" "$dir"/{a,b,c}.rb 2>&1 #"$dir/stderr.txt"
# grep 'No errors! Great job.' "$dir/stderr.txt"
# grep 'types.input' "$dir/stderr.txt"
