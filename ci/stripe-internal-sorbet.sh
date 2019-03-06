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
bazel query 'tests(//...) except attr("tags", "manual", //...)' | while read -r line; do
    path="${line/://}"
    path="${path#//}"
    cp "bazel-testlogs/$path/test.xml" /log/junit/"${path//\//_}.xml"
done

if [ "$err" -ne 0 ]; then
    exit "$err"
fi

bazel build main:sorbet $args
(
    cd rbi-generation
    rbenv exec bundle
    PATH="$PATH:$(bazel info bazel-bin $args)/main/"
    rbenv exec bundle exec rake test
)
