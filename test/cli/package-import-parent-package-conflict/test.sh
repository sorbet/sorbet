cd test/cli/package-import-parent-package-conflict || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1


