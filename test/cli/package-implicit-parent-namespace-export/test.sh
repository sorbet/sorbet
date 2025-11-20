cd test/cli/package-implicit-parent-namespace-export || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1
