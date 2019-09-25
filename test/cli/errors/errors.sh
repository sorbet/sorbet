#!/bin/bash

main/sorbet --silence-dev-message test/cli/errors/errors.rb 2>&1

CACHE=$(mktemp -d)

censor_payload_locs() {
  sed -e 's/https:\/\/github.com\/sorbet\/sorbet.*: /...: /' | \
    sed -e 's/^\( *\)[0-9]* |/\1.. |/'
}

main/sorbet --silence-dev-message --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1 | censor_payload_locs
main/sorbet --silence-dev-message --cache-dir="$CACHE" test/cli/errors/errors.rb 2>&1 | censor_payload_locs
