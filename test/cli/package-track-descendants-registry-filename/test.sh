cd test/cli/package-track-descendants-registry-filename || exit 1

echo "--- custom registry filename"
../../../main/sorbet --silence-dev-message --censor-for-snapshot-tests --sorbet-packages --max-threads=0 \
  --descendants-registry-filename=registry.rbi . 2>&1

echo "--- path instead of a file name"
../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 \
  --descendants-registry-filename=b/registry.rbi . 2>&1

echo "--- not an RBI file"
../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 \
  --descendants-registry-filename=registry.rb . 2>&1

echo "--- without --sorbet-packages"
../../../main/sorbet --silence-dev-message --max-threads=0 \
  --descendants-registry-filename=registry.rbi . 2>&1
