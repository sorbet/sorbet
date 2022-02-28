cd test/cli/package-secondary-test-namespace-invalid || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --secondary-test-package-namespaces=Critic:: --stripe-mode . 2>&1
../../../main/sorbet --silence-dev-message --stripe-packages --secondary-test-package-namespaces=critic --stripe-mode . 2>&1
../../../main/sorbet --silence-dev-message --stripe-packages --secondary-test-package-namespaces=^ --stripe-mode . 2>&1
