cd test/cli/package-skip-import-visibility-check-for || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --skip-package-import-visibility-check-for=Project::Root --max-threads=0 . 2>&1


