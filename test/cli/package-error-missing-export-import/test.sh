cd test/cli/package-error-missing-export-import || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --sorbet-packages-hint-message="Try running generate-packages.sh" \
  --max-threads=0 other missing_import 2>&1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --sorbet-packages-hint-message="Try running generate-packages.sh" \
  --max-threads=0 other imported_as_test 2>&1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --sorbet-packages-hint-message="Try running generate-packages.sh" \
  --max-threads=0 other imported_as_test_unit 2>&1
