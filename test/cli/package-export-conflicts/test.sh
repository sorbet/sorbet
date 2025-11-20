cd test/cli/package-export-conflicts || exit 1

../../../main/sorbet --max-threads=0 --silence-dev-message --sorbet-packages . 2>&1
