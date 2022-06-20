cd test/cli/package-disallow-rbi-only-exports || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --max-threads=0 . 2>&1

