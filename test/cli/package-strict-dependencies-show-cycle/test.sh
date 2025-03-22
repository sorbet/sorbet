cd test/cli/package-strict-dependencies-show-cycle || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --packager-layers . 2>&1
