#!/bin/bash
set -eux

DIR=./pay-server

if [ ! -d $DIR ]; then
    echo "$DIR doesn't exist"
    exit 1
fi

cd $DIR
if [ ! -f "../ci/stripe-internal-ruby-typer-pay-server-sha" ]; then
    echo "ci/stripe-internal-ruby-typer-pay-server-sha doesn't exist"
    exit 1
fi
git checkout "$(cat ../ci/stripe-internal-ruby-typer-pay-server-sha)"
cd -

# This is what we'll ship to our users
bazel build main:ruby-typer --config=unsafe -c opt

ALL_FILES="$(mktemp)"
find $DIR -name *.rb | sort > "$ALL_FILES"

# Make sure these specific files are typed
for f in $(cat ci/stripe-internal-ruby-typer-pay-server-typechecked); do
    echo "# @typed" >> "$f"
done

OUT=$(mktemp)
./bazel-bin/main/ruby-typer @"$ALL_FILES" > "$OUT" 2>&1
if [ -s "$OUT" ]; then
    cat "$OUT"
    exit 1
fi

# Make sure we don't crash on all of pay-server with ASAN on
bazel build main:ruby-typer -c opt

# Disable leak sanatizer. Does not work in docker
# https://github.com/google/sanitizers/issues/764
ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=verbosity=1:log_threads=1 /usr/bin/time -v ./bazel-bin/main/ruby-typer --quiet --error-stats --typed=always @"$ALL_FILES"
