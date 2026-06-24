cd test/cli/package-bad-imports || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 --print=symbol-table . 2>&1
