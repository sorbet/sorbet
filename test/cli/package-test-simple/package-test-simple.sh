cd test/cli/package-test-simple || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --stripe-mode . 2>&1

echo
echo "TODO(ngroman) Move this error enforcement into Sorbet"
