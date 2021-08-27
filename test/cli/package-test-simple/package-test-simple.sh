cd test/cli/package-test-simple || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages . 2>&1
