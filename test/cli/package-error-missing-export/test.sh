cd test/cli/package-error-missing-export || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --sorbet-packages-hint-message="Try running generate-packages.sh" \
  --max-threads=0 other use_other 2>&1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --sorbet-packages-hint-message="Try running generate-packages.sh" \
  --packager-layers=lib,app \
  --max-threads=0 app_false_cycle_package use_app_false_cycle_package 2>&1
