cd test/cli/package-layers-disabled-no-layers || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --max-threads=0 . 2>&1
