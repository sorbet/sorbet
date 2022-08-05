#!/bin/sh
dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

set -e
metrics="$dir/metrics.json"

# attr_accessor.rb relies on the Rewriter pass to typecheck properly. We
# cache it using a normal run, and then re-run without Rewriter passes and
# verify that it fails (i.e. that it isn't re-using cached DSL-passed
# source). We re-run twice to exercise both cached and uncached paths.
main/sorbet \
    --censor-for-snapshot-tests --silence-dev-message \
    --cache-dir "$dir/" \
    test/cli/cache-dsl/attr_accessor.rb 2>&1

for pass in uncached cached; do
    if main/sorbet \
           --censor-for-snapshot-tests --silence-dev-message \
           --cache-dir "$dir/" \
           --skip-rewriter-passes \
           --metrics-prefix=cache-dsl \
           --metrics-file="$metrics" \
           test/cli/cache-dsl/attr_accessor.rb 2>&1; then
        echo "FAILED: skip-rewriter-passes should generate an error"
    fi
    uncached=
    if grep -q "types.input.files.kvstore.miss" "$metrics"; then
        uncached=1
    fi
    case "$pass,$uncached" in
        cached,) ;;
        uncached,1) ;;
        *)
            echo "FAILED: pass=$pass mismatch on expected-cache-miss"
            cat "$metrics"
            ;;
    esac
done
