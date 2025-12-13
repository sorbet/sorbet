#!/usr/bin/env bash

# If we have gtar installed (darwin), use that instead
if hash gtar 2>/dev/null; then
    alias tar=gtar
fi

hermetic_tar() {
    SRC="$1"
    OUT="$2"
    # Check if our tar supports --sort (we assume --sort support implies --mtime support)
    if [[ $(tar --sort 2>&1) =~ requires\ an\ argument ]]; then
        tar --sort=name --owner=0 --group=0 --numeric-owner --mtime='UTC 1970-01-01 00:00' -h -C "$SRC" -rf "$OUT" .
    elif [[ $(tar --mtime 2>&1) =~ requires\ an\ argument ]]; then
        (cd "$SRC" && find . -print0) | \
          LC_ALL=C sort -z | \
          tar -h -C "$SRC" --no-recursion --null -T - --owner=0 --group=0 --numeric-owner --mtime='UTC 1970-01-01 00:00' -rf "$OUT" .
    else
        # Oh well, no hermetic tar for you
        tar -h -C "$SRC" -rf "$OUT" .
    fi
}
