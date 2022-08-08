#!/bin/sh
#!/bin/bash
main/sorbet --censor-for-snapshot-tests --silence-dev-message test/cli/suggest-not-stub/suggest-not-stub.rb 2>&1
