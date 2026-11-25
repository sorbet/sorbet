cd test/cli/package-private || exit 1

../../../main/sorbet --silence-dev-message --censor-for-snapshot-tests --sorbet-packages --max-threads=0 . 2>&1


