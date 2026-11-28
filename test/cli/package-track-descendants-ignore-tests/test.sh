cd test/cli/package-track-descendants-ignore-tests || exit 1

echo "--- without --descendants-registry-ignore-tracking-in-tests"
../../../main/sorbet --silence-dev-message --censor-for-snapshot-tests --sorbet-packages --max-threads=0 . 2>&1

echo "--- with --descendants-registry-ignore-tracking-in-tests"
../../../main/sorbet --silence-dev-message --censor-for-snapshot-tests --sorbet-packages --max-threads=0 \
  --descendants-registry-ignore-tracking-in-tests . 2>&1

echo "--- without --sorbet-packages"
../../../main/sorbet --silence-dev-message --max-threads=0 --descendants-registry-ignore-tracking-in-tests . 2>&1
