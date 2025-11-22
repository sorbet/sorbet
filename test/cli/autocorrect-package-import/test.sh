#!/usr/bin/env bash

cwd="$(pwd)"
tmp="$(mktemp -d)"

mkdir -p "${tmp}"/{a,b}
cp "$cwd/test/cli/autocorrect-package-import/a/__package.rb" "$tmp/a"
cp "$cwd/test/cli/autocorrect-package-import/a/foo.rb" "$tmp/a"
cp "$cwd/test/cli/autocorrect-package-import/b/__package.rb" "$tmp/b"
cp "$cwd/test/cli/autocorrect-package-import/b/foo.rb" "$tmp/b"

cd "$tmp" || exit 1

if "$cwd/main/sorbet" --silence-dev-message --sorbet-packages -a . 2>&1; then
    echo "Expected to fail!"
    exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat a/__package.rb

rm autocorrect-abstract.rb
