main/sorbet --silence-dev-message test/cli/enable-experimental-requires-ancestor/enable-experimental-requires-ancestor.rb 2>&1

echo --------------------------------------------------------------------------

main/sorbet --silence-dev-message --enable-experimental-requires-ancestor \
  test/cli/enable-experimental-requires-ancestor/enable-experimental-requires-ancestor.rb 2>&1
