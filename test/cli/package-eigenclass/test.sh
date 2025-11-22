cd test/cli/package-eigenclass || exit 0

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1

