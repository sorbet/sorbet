cd test/cli/package-test-simple || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --secondary-test-namespaces=Critic --stripe-mode . 2>&1
