#!/bin/bash

# ----- Setup -----------------------------------------------------------------

set -eux

# shellcheck disable=SC1091
source /usr/stripe/bin/docker/stripe-init-build

source ci/pay-server-common.bash

# ----- Build and publish a version of Sorbet with sanitizers -----------------

export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
export LSAN_OPTIONS=verbosity=1:log_threads=1

/usr/local/bin/junit-script-output \
    sanitized-build \
    bazel build main:sorbet --config=stripeci --config=release-sanitized-linux

cp bazel-bin/main/sorbet /build/bin/sorbet.san


# ----- Set up symbolizer -----------------------------------------------------

# (This has to be after the bazel build)

ASAN_SYMBOLIZER_PATH="$(bazel info output_base)/external/llvm_toolchain/bin/llvm-symbolizer"
ASAN_SYMBOLIZER_PATH="$(realpath "$ASAN_SYMBOLIZER_PATH")"
export ASAN_SYMBOLIZER_PATH
$ASAN_SYMBOLIZER_PATH -version


# ----- Set up pay-server -----------------------------------------------------

pushd "$PAY_SERVER"
initialize_pay_server


# ----- Stress test anything we can think of  ---------------------------------

# The goal here is no ENFORCE failures, exceptions, or sanitizer crashes.
# There will be lots of type errors, because we're passing the --typed= flag.
#
# Specifically, the --stress-incremntal-resolver flag *might* introduce new
# type errors that wouldn't show up in the normal pipeline, even after the
# typed overrides.

/usr/local/bin/junit-script-output \
    typecheck-sanitized \
    /usr/bin/time -o "$TIMEFILE" \
    ./scripts/bin/typecheck \
    --suppress-non-critical --typed=strict --quiet \
    --stress-incremental-resolver
cat "$TIMEFILE"

if [ "$RECORD_STATS" ]; then
    t_user="$(grep user "$TIMEFILE" | cut -d ' ' -f 2)"
    t_wall="$(grep wall "$TIMEFILE" | cut -d ' ' -f 2)"
    veneur-emit -hostport veneur-srv.service.consul:8200 -debug -timing "$t_wall"s -name ruby_typer.payserver.full_run.seconds
    veneur-emit -hostport veneur-srv.service.consul:8200 -debug -timing "$t_user"s -name ruby_typer.payserver.full_run.cpu_seconds
fi
