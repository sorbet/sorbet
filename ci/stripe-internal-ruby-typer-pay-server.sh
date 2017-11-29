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

bazel build main:ruby-typer -c opt

# Disable leak sanatizer. Does not work in docker
# https://github.com/google/sanitizers/issues/764
ASAN_OPTIONS=detect_leaks=0

# Make sure these files have 0 type errors
TMP=$(mktemp)
ASAN_OPTIONS=$ASAN_OPTIONS ./bazel-bin/main/ruby-typer @ci/stripe-internal-ruby-typer-pay-server-typechecked > $TMP 2>&1
if [ -s "$TMP" ]; then
    exit 1
fi

# Make sure we don't crash on all of pay-server
TMP="$(mktemp)"
find $DIR -name *.rb | sort > $TMP
ASAN_OPTIONS=$ASAN_OPTIONS LSAN_OPTIONS=verbosity=1:log_threads=1 ./bazel-bin/main/ruby-typer --quiet --error-stats @$TMP
