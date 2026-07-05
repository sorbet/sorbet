#!/bin/bash

censor_payload_locs() {
  sed -e 's/https:\/\/github.com\/sorbet\/sorbet.*: /...: /' | \
    sed -e 's/^\( *\)[0-9]* |/\1.. |/'
}

main/sorbet --censor-for-snapshot-tests --silence-dev-message test/cli/errors/errors.rb 2>&1 | censor_payload_locs

CACHE=$(mktemp -d)

main/sorbet --censor-for-snapshot-tests --silence-dev-message --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1 | censor_payload_locs
main/sorbet --censor-for-snapshot-tests --silence-dev-message --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1 | censor_payload_locs
