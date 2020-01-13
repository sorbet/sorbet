#!/bin/bash

set -e

cd "$(dirname "$0")"
cd ../..

grep -l -r test -e '# disable-fast-path: true' | while read -r FILE; do
    if [[ "$FILE" == *.rb && ! "$FILE" =~ .*__[0-9].rb ]]; then
        BASENAME=${FILE%.rb}
        TESTNAME="//test:test_LSPTests/${BASENAME#test/}"
        sed -i .backup -e '/^# disable-fast-path: true$/d' "$FILE"
        if bazel test "$TESTNAME"; then
            echo "$TESTNAME succeeds now; leaving as-is"
            rm "$FILE.backup"
        else
            mv "$FILE.backup" "$FILE"
        fi
    fi
done
