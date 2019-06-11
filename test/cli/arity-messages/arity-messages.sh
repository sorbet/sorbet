#!/bin/bash

set -e

main/sorbet --silence-dev-message test/cli/arity-messages/arity-messages.rb 2>&1
