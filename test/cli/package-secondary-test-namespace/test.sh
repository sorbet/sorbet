cd test/cli/package-secondary-test-namespace || exit 0

../../../main/sorbet --silence-dev-message --stripe-packages --uniquely-defined-behavior . 2>&1

