cd test/cli/package-prefix-enforcement || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages . 2>&1
