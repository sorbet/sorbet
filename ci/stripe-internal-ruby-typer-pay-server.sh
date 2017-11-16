#!/bin/bash
set -eux

DIR=./pay-server

if [ ! -d $DIR ]; then
    echo "$DIR doesn't exist"
    exit 1
fi

cd $DIR
if [ `git rev-parse --abbrev-ref HEAD` != "master-passing-tests" ]; then
    echo "pay-server not on master-passing-tests branch"
    exit 1
fi
cd -

bazel build main:ruby-typer

BLACKLIST="\(api/lib/api_param/pagination.rb\|lib/fees/charge_based_fee_rules.rb\|lib/interchange/framework/debug.rb\|lib/interchange/framework/property_parameter_comparison.rb\|lib/interchange/framework/rules/gives_result.rb\|lib/db.rb\)"

# Disable leak sanatizer. Does not work in docker
# https://github.com/google/sanitizers/issues/764
find $DIR -name *.rb | sort | grep -v "$BLACKLIST" | ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=verbosity=1:log_threads=1 xargs ./bazel-bin/main/ruby-typer -q
