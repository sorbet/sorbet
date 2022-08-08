#!/bin/bash
main/sorbet --censor-for-snapshot-tests --silence-dev-message --dir test/cli/remove-path-prefix-https --remove-path-prefix https://git.corp.stripe.com/stripe-internal/sorbet/tree/master/ 2>&1
