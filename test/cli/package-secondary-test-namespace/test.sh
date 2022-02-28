cd test/cli/package-secondary-test-namespace || exit 0

../../../main/sorbet --silence-dev-message --stripe-packages --secondary-test-package-namespaces=Critic,Minitest --stripe-mode . 2>&1

