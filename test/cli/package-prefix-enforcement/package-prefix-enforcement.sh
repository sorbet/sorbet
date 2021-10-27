cd test/cli/package-prefix-enforcement || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --secondary-test-package-namespaces=Critic --max-threads=0 . 2>&1
