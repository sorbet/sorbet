#!/bin/bash
set -e

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 path/to/typedruby COMMIT" >&2
    exit 1
fi

path=$1
commit=$2

if ! sha=$(git -C "$path" rev-parse -q --verify "$commit^{commit}"); then
    echo "Commit '$commit' not found" >&2
    exit 1
fi

root="$(dirname "$0")/../.."
git rm --quiet -r "$root/third_party/parser"

mkdir -p "$root/third_party/parser"

git -C "$path" archive "$commit:parser" | tar -C "$root/third_party/parser" -x

git add "$root/third_party/parser"
git commit "$root/third_party/parser" -m "Import parser from typedruby@${sha}."
