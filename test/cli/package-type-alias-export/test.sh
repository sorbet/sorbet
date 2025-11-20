cd test/cli/package-type-alias-export || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages . 2>&1
