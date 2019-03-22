#!/bin/bash
main/sorbet --silence-dev-message test/cli/remove-path-prefix-https --remove-path-prefix https://git.corp.stripe.com/stripe-internal/sorbet/tree/master/ 2>&1
