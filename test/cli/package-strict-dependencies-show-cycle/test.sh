cd test/cli/package-strict-dependencies-show-cycle || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --packager-layers --max-threads=0 . 2>&1
