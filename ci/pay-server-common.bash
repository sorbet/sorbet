# No shebang, because this file is meant to be sourced.
# Instead, we set the .bash file extension.
#
# This file sets up our two pay-server jobs to be ~the same.
# It's mostly defining global variables, creating folders, etc.
#
# Since this file lives in the same repo, shellcheck is able to follow the path
# and know which globals we define even in files that source this file.

export TIME='cmd: "%C"
wall: %e
user: %U
system: %S
maxrss: %M
'

OUT="$(mktemp)"
export OUT
TIMEFILE="$(mktemp)"
export TIMEFILE

SORBET_CACHE_DIR=$(mktemp -d)
mkdir -p "${SORBET_CACHE_DIR}/bin"
export SORBET_CACHE_DIR
RUBY_TYPER_CACHE_DIR=$SORBET_CACHE_DIR
export RUBY_TYPER_CACHE_DIR

GIT_SHA=$(git rev-parse HEAD)

SORBET_DIR="$(pwd)"
PAY_SERVER="$SORBET_DIR/pay-server"
if [ ! -d "$PAY_SERVER" ]; then
    echo "$PAY_SERVER doesn't exist"
    exit 1
fi

if [ ! -f "ci/stripe-internal-sorbet-pay-server-sha" ]; then
    echo "ci/stripe-internal-sorbet-pay-server-sha doesn't exist"
    exit 1
fi
PAY_SERVER_SHA="$(cat ci/stripe-internal-sorbet-pay-server-sha)"

mkdir -p /build/bin

RECORD_STATS=
if [ "$GIT_BRANCH" = "master" ] || [[ "$GIT_BRANCH" == integration-* ]]; then
    RECORD_STATS=1
    echo Will submit metrics for "$GIT_SHA"
fi
export RECORD_STATS

initialize_pay_server() {
    # Uncomment this to build against a non-master pay-server branch
    # git fetch origin "$PAY_SERVER_SHA"
    git checkout "$PAY_SERVER_SHA"

    # Force the sorbet binary into existence at the SHA pay-server expects it.
    # This circumvents the logic in sorbet/scripts/sorbet that fetches a
    # passing build asset from S3.
    SORBET_MASQUERADE_SHA="$(cat sorbet/sorbet.sha)"
    mkdir -p "$SORBET_CACHE_DIR/$SORBET_MASQUERADE_SHA/bin"
    ln -s "$SORBET_DIR/bazel-bin/main/sorbet" "$SORBET_CACHE_DIR/$SORBET_MASQUERADE_SHA/bin"

    eval "$(rbenv init -)"
    stripe-deps-ruby --without ci_ignore

    # Clobber our .bazelrc so pay-server's bazel doesn't see it.
    mv "$SORBET_DIR/.bazelrc" "$SORBET_DIR/.bazelrc-moved"

    # Unset JENKINS_URL so pay-server acts like it's running in dev, not
    # CI, and runs its own `bazel` invocations
    env -u JENKINS_URL rbenv exec bundle exec rake build:autogen:SorbetFileListStep

    # restore bazelrc
    mv "$SORBET_DIR/.bazelrc-moved" "$SORBET_DIR/.bazelrc"
}
