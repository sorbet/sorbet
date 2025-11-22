cd test/cli/package-error-unresolved-export || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --sorbet-packages-hint-message="Try running generate-packages.sh" \
  --max-threads=0 . 2>&1
