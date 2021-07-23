cd test/cli/package-chalk || exit 1

../../../main/sorbet --silence-dev-message  --stripe-packages . 2>&1
