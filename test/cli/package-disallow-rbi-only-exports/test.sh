cd test/cli/package-disallow-rbi-only-exports || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --package-skip-rbi-export-enforcement=./skip-rbi-export-enforcement/ --max-threads=0 . 2>&1

