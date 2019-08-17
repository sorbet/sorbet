main/sorbet \
  --silence-dev-message \
  -p symbol-table-json \
  test/cli/symbol-table-json/symbol-table-json.rb 2>&1 | \
    sed -e 's/^\( *\)"id": [0-9]*/\1"id": <redacted>/' | \
    sed -e 's/^\( *\)"superClass": [0-9]*/\1"id": <redacted>/' | \
    sed -e 's/^\( *\)"aliasTo": [0-9]*/\1"aliasTo": <redacted>/'
