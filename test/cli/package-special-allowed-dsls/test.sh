cd test/cli/package-special-allowed-dsls || exit 0

../../../main/sorbet --silence-dev-message --stripe-packages . 2>&1

