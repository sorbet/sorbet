cd test/cli/package-disallow-enum-value-exports || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1


