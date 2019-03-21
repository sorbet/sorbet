#!/bin/bash
# shellcheck disable=SC2086
set -eux

# This is the easiest (only?) way to tell bazel not to recurse into
# this subdirectory.
echo '
local_repository(
    name = "stripe_pay_server",
    path = "pay-server",
)
' >> WORKSPACE

./tools/scripts/ci_checks.sh
rbenv exec gem install bundler
(
    cd runtime
    rbenv exec bundle
    rbenv exec bundle exec rake test
)

err=0

args="--config=test-sanitized-linux --config=stripeci"
# Disable leak sanatizer. Does not work in docker
# https://github.com/google/sanitizers/issues/764
bazel test //... $args --test_output=errors || err=$?

mkdir -p /log/junit

# Don't spew into the logs for every single text.xml file being copied
set +x

bazel query 'tests(//...) except attr("tags", "manual", //...)' | while read -r line; do
    path="${line/://}"
    path="${path#//}"
    cp "bazel-testlogs/$path/test.xml" /log/junit/"${path//\//_}.xml"
done
set -x

if [ "$err" -ne 0 ]; then
    exit "$err"
fi

export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
export LSAN_OPTIONS=verbosity=1:log_threads=1

bazel build main:sorbet $args
cp "$(bazel info bazel-bin $args)/main/sorbet" bin/sorbet

/usr/local/bin/junit-script-output \
  rbi-generation-typecheck \
  bin/sorbet ./rbi-generation

(
    cd rbi-generation
    gem build sorbet-rbi-generation.gemspec
    gem install sorbet-rbi-generation-*.gem
)
gem build sorbet.gemspec
gem install --no-wrappers sorbet*.gem
(
    cd rbi-generation
    rbenv exec bundle
    rbenv exec bundle exec rake test
)
