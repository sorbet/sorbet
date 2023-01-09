cd test/cli/package-implicit-parent-namespace-export || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --max-threads=0 . 2>&1
