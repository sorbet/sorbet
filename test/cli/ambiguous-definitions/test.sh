#!/bin/bash

set -e

main/sorbet --silence-dev-message test/cli/ambiguous-definitions/ 2>&1

