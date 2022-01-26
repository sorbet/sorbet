cd test/cli/package-error-no-export-for-test || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages \
  --max-threads=0 . 2>&1
