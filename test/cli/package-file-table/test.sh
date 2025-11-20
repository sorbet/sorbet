cd test/cli/package-file-table || exit 1

../../../main/sorbet --print=file-table-json --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1
