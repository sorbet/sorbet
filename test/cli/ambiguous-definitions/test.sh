#!/bin/bash

set -euo pipefail

main/sorbet --silence-dev-message test/cli/ambiguous-definitions/ 2>&1

