cd test/cli/package-error-no-export-for-test || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --max-threads=0 . 2>&1
