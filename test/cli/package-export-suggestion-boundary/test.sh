cd test/cli/package-export-suggestion-boundary || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1
