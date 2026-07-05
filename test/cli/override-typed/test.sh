#!/bin/bash
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed-override=test/cli/override-typed/override-typed.yaml test/cli/override-typed/override-typed.rb 2>&1
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed-override=test/cli/override-typed/override-typed.yaml ./test/cli/override-typed/override-typed.rb 2>&1
main/sorbet --censor-for-snapshot-tests --silence-dev-message --typed-override=test/cli/override-typed/override-typed.yaml . --ignore external 2>&1
