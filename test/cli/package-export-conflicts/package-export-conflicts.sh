cd test/cli/package-export-conflicts || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages . 2>&1
