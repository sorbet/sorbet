cd test/cli/package-test-simple || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 --uniquely-defined-behavior --sorbet-packages-hint-message="RUN SCRIPT HINT" . 2>&1
