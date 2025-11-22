cd test/cli/package-root || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages . 2>&1
