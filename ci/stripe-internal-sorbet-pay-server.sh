#!/bin/bash

# ----- Setup -----------------------------------------------------------------

set -eux

# shellcheck disable=SC1091
source /usr/stripe/bin/docker/stripe-init-build

source ci/pay-server-common.bash


# ----- Build and publish Linux release build ---------------------------------

/usr/local/bin/junit-script-output \
    release-build \
    bazel build main:sorbet --config=stripeci --config=release-linux

# Validate the version  (we can't do this as a CLI test because the snapshot
# test would fail when not building release builds)
VERSION=$(./bazel-bin/main/sorbet --version)
EXPECTED_VERSION='Ruby Typer rev [0-9]* git [0-9a-f]{40} built on 201.* GMT with debug symbols'
if ! [[ $VERSION =~ $EXPECTED_VERSION ]]; then
    git status
    echo "Bad Version: $VERSION"
    exit 1
fi

cp bazel-bin/main/sorbet /build/bin


# ----- Set up pay-server -----------------------------------------------------

pushd "$PAY_SERVER"
initialize_pay_server


# ----- Run like our users will (scripts/bin/typecheck, fast, no debug) -------

/usr/local/bin/junit-script-output \
    typecheck-uncached \
    /usr/bin/time -o "$TIMEFILE" ./scripts/bin/typecheck 2>&1 | tee "$OUT"
if [ "$(cat "$OUT")" != "No errors! Great job." ]; then
    exit 1
fi
cat "$TIMEFILE"

if [ "$RECORD_STATS" ]; then
    t_user="$(grep user "$TIMEFILE" | cut -d ' ' -f 2)"
    t_wall="$(grep wall "$TIMEFILE" | cut -d ' ' -f 2)"
    veneur-emit -hostport veneur-srv.service.consul:8200 -debug -timing "$t_wall"s -name ruby_typer.payserver.prod_run.seconds
    veneur-emit -hostport veneur-srv.service.consul:8200 -debug -timing "$t_user"s -name ruby_typer.payserver.prod_run.cpu_seconds
fi


# ----- Run twice more to exercise caching ------------------------------------

/usr/local/bin/junit-script-output \
    typecheck-cached \
    /usr/bin/time -o "$TIMEFILE" ./scripts/bin/typecheck 2>&1 | tee "$OUT"
if [ "$(cat "$OUT")" != "No errors! Great job." ]; then
    exit 1
fi
cat "$TIMEFILE"

/usr/local/bin/junit-script-output \
    typecheck-final \
    /usr/bin/time -o "$TIMEFILE" ./scripts/bin/typecheck 2>&1 | tee "$OUT"
if [ "$(cat "$OUT")" != "No errors! Great job." ]; then
    exit 1
fi
cat "$TIMEFILE"


# ----- Make sure the order of the file list does not matter ------------------

get_seeded_random() {
    seed="$1"
    openssl enc -aes-256-ctr -pass pass:"$seed" -nosalt \
        </dev/zero 2>/dev/null
}

for i in {1..10}; do
  SAVED_FILE_LIST=$(mktemp)
  FILE_LIST=build/sorbet_file_lists/RUBY_TYPER
  cp "$FILE_LIST" "$SAVED_FILE_LIST"
  SEED=$(cat "$SORBET_DIR/ci/stripe-internal-sorbet-pay-server-random")
  sort --random-source=<(get_seeded_random "$SEED$i") -R "$SAVED_FILE_LIST" > "$FILE_LIST"

  /usr/local/bin/junit-script-output \
      "typecheck-random-$i" \
      /usr/bin/time -o "$TIMEFILE" ./scripts/bin/typecheck 2>&1 | tee "$OUT"
  if [ "$(cat "$OUT")" != "No errors! Great job." ]; then
     exit 1
  fi

  cat "$TIMEFILE"
  mv "$SAVED_FILE_LIST" "$FILE_LIST"
done


# ----- Still no type errors, even in incremental resolver mode ---------------

/usr/local/bin/junit-script-output \
    typecheck-incremental-resolver \
    /usr/bin/time -o "$TIMEFILE" ./scripts/bin/typecheck --stress-incremental-resolver 2>&1 | tee "$OUT"
if [ "$(cat "$OUT")" != "No errors! Great job." ]; then
    exit 1
fi
cat "$TIMEFILE"



# ----- Run a --typed=strict run for metrics only -----------------------------

/usr/local/bin/junit-script-output \
    typecheck-metrics \
    /usr/bin/time -o "$TIMEFILE" \
    ./scripts/bin/typecheck \
    --suppress-non-critical --typed=strict --quiet \
    --counters \
    --statsd-host=veneur-srv.service.consul \
    --statsd-prefix=ruby_typer.payserver \
    --metrics-file=metrics.json \
    --metrics-prefix=ruby_typer.payserver \
    --metrics-repo=stripe-internal/sorbet \
    --metrics-sha="$GIT_SHA"

if [ "$RECORD_STATS" ]; then
    LOG_DIR="/log/persisted/$(date "+%Y%m%d")/$PAY_SERVER_SHA/"
    mkdir -p "$LOG_DIR"
    cp metrics.json "$LOG_DIR"
fi


# ----- Ship a version with debug assertions and counters ---------------------

# This is way down here instead of up with the previous bazel build because we
# want to fail sooner if there's a type error on pay-server.
#
# We don't actually care about running this over pay-server;
# the build with sanitizers also runs with debug checks.

popd

/usr/local/bin/junit-script-output \
    release-debug-build \
    bazel build main:sorbet --config=stripeci --config=release-debug-linux

cp bazel-bin/main/sorbet /build/bin/sorbet.dbg


