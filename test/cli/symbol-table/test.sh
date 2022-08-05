main/sorbet \
  --censor-for-snapshot-tests --silence-dev-message \
  -p symbol-table \
  test/cli/symbol-table/symbol-table.rb 2>&1
