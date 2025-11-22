cd test/cli/package-top-level-inheritance || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages . 2>&1

