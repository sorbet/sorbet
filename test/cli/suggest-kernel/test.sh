#!/bin/sh
#!/bin/bash

censor_payload_locs() {
  sed -e 's/https:\/\/github.com\/sorbet\/sorbet.*: /...: /' | \
    sed -e 's/^\( *\)[0-9]* |/\1.. |/'
}

main/sorbet --censor-for-snapshot-tests --silence-dev-message test/cli/suggest-kernel/suggest-kernel.rb 2>&1 | censor_payload_locs
