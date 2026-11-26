#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/autocorrect-missing-else/autocorrect-missing-else.rb"

tmp="$(mktemp -d)"

for flags in "--suggest-unsafe -a" "-a"; do
  (
    cp "$infile" "$tmp"

    cd "$tmp" || exit 1
    echo "--- $flags"
    echo
    # shellcheck disable=SC2086
    if "$cwd/main/sorbet" --silence-dev-message $flags autocorrect-missing-else.rb > /dev/null 2>&1; then
      echo "Expected to fail!"
      exit 1
    fi

    cat autocorrect-missing-else.rb

    # Make sure that the autocorrected file parses, and see which errors are left.
    echo
    echo "--- $flags, after autocorrecting"
    echo
    "$cwd/main/sorbet" --silence-dev-message autocorrect-missing-else.rb 2>&1

    rm autocorrect-missing-else.rb
  )
  echo
done

rm -r "$tmp"
