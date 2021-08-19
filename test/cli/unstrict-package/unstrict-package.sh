cd test/cli/unstrict-package || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages . 2>&1
