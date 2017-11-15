#!/bin/bash
set -eux
    
DIR=/cache/pay-server

if [ ! -d $DIR/.git ]; then
    rm -r $DIR
fi
if [ ! -d $DIR ]; then
    git clone git@git.corp.stripe.com:stripe-internal/pay-server.git $DIR
fi
cd $DIR
git fetch
git checkout master-passing-tests
git rebase origin/master-passing-tests
cd -

bazel build main:ruby-typer

BLACKLIST="\(api/lib/api_param/pagination.rb\|lib/fees/charge_based_fee_rules.rb\|lib/interchange/framework/debug.rb\|lib/interchange/framework/property_parameter_comparison.rb\|lib/interchange/framework/rules/gives_result.rb\|lib/db.rb\)"

# Disable leak sanatizer. Does not work in docker
# https://github.com/google/sanitizers/issues/764
find $DIR -name *.rb | sort | grep -v "$BLACKLIST" | ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=verbosity=1:log_threads=1 xargs ./bazel-bin/main/ruby-typer -v
