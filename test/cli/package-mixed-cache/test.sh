#!/usr/bin/env bash

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-mixed-cache || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1


# Create the cache in monolithic mode
"$cwd/main/sorbet" --silence-dev-message --sorbet-packages --max-threads=0 --cache-dir=cache . 2>&1

# Consume the cache in package-directed mode
"$cwd/main/sorbet" --silence-dev-message --sorbet-packages --experimental-package-directed --max-threads=0 --cache-dir=cache . 2>&1

rm -rf cache

# Create the cache in package-directed mode
"$cwd/main/sorbet" --silence-dev-message --sorbet-packages --experimental-package-directed --max-threads=0 --cache-dir=cache . 2>&1

# Consume the cache in monolithic mode
"$cwd/main/sorbet" --silence-dev-message --sorbet-packages --max-threads=0 --cache-dir=cache . 2>&1
