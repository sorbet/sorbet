cd test/cli/package-error-missing-export || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages \
  --stripe-packages-hint-message="Try running generate-packages.sh" \
  --max-threads=0 other use_other 2>&1
