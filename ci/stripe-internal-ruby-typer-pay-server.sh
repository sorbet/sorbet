#!/bin/bash
set -eux

DIR=./pay-server

export TIME='cmd: "%C"
wall: %e
user: %U
system: %S
maxrss: %M
'

# This is what we'll ship to our users
bazel build main:ruby-typer -c opt --config=lto --stamp

mkdir -p /build/bin
cp bazel-bin/main/ruby-typer /build/bin

PATH=$PATH:"$(pwd)/bazel-bin/main/"
export PATH

if [ ! -d $DIR ]; then
    echo "$DIR doesn't exist"
    exit 1
fi
cd $DIR

if [ ! -f "../ci/stripe-internal-ruby-typer-pay-server-sha" ]; then
    echo "ci/stripe-internal-ruby-typer-pay-server-sha doesn't exist"
    exit 1
fi
PAY_SERVER_SHA="$(cat ../ci/stripe-internal-ruby-typer-pay-server-sha)"
git checkout "$PAY_SERVER_SHA"

# Make sure these specific files are typed
while IFS= read -r f; do
    echo "# @typed" >> "$f"
done < ../ci/stripe-internal-ruby-typer-pay-server-typechecked

RECORD_STATS=
if [ "$GIT_BRANCH" == "master" ] || [[ "$GIT_BRANCH" == integration-* ]]; then
    RECORD_STATS=1
fi

OUT=$(mktemp)
TIMEFILE1=$(mktemp)

/usr/bin/time -o "$TIMEFILE1" ./scripts/ruby-types/typecheck 2>&1 | tee "$OUT"
if [ -s "$OUT" ]; then
    exit 1
fi

cat "$TIMEFILE1"

if [ "$RECORD_STATS" ]; then
    t_user="$(grep user "$TIMEFILE1" | cut -d ' ' -f 2)"
    t_wall="$(grep wall "$TIMEFILE1" | cut -d ' ' -f 2)"
    veneur-emit -hostport veneur-srv.service.consul:8200 -debug -timing "$t_wall"s -name ruby_typer.payserver.prod_run.seconds
    veneur-emit -hostport veneur-srv.service.consul:8200 -debug -timing "$t_user"s -name ruby_typer.payserver.prod_run.cpu_seconds
fi



# Run 2: Make sure we don't crash on all of pay-server with ASAN on

(
    cd -
    bazel build main:ruby-typer -c opt --config=ci --config=sanitize
)

TIMEFILE2=$(mktemp)

# Disable leak sanatizer. Does not work in docker
# https://github.com/google/sanitizers/issues/764
env ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-4.0/bin/llvm-symbolizer \
    ASAN_OPTIONS=detect_leaks=0:detect_odr_violation=0 \
    LSAN_OPTIONS=verbosity=1:log_threads=1 \
    /usr/bin/time -o "$TIMEFILE2" \
    ./scripts/ruby-types/typecheck --quiet --typed=always --statsd-host=veneur-srv.service.consul --statsd-prefix=ruby_typer.payserver --counters --metrics-file=metrics.json --metrics-prefix=ruby_typer.payserver. --metrics-repo=payserver --metrics-sha="$PAY_SERVER_SHA"

cat "$TIMEFILE2"

if [ "$RECORD_STATS" ]; then
    t_user="$(grep user "$TIMEFILE2" | cut -d ' ' -f 2)"
    t_wall="$(grep wall "$TIMEFILE2" | cut -d ' ' -f 2)"
    veneur-emit -hostport veneur-srv.service.consul:8200 -debug -timing "$t_wall"s -name ruby_typer.payserver.full_run.seconds
    veneur-emit -hostport veneur-srv.service.consul:8200 -debug -timing "$t_user"s -name ruby_typer.payserver.full_run.cpu_seconds
    mkdir -p /log/persisted
    cp metrics.json /log/persisted
fi
