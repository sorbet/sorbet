#!/bin/bash

set -e

main/sorbet --silence-dev-message test/cli/type-member-template/type-member-template.rb 2>&1
