cd test/cli/package-alias-namespace-recovery || exit 1

echo 'Monolithic:'
../../../main/sorbet --silence-dev-message --censor-for-snapshot-tests --sorbet-packages --max-threads=0 . 2>&1

echo 'Package-directed:'
../../../main/sorbet --silence-dev-message --censor-for-snapshot-tests --sorbet-packages --experimental-package-directed --max-threads=0 . 2>&1
