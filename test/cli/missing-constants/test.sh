#!/bin/bash

set -euo pipefail
main/sorbet --silence-dev-message -p missing-constants test/cli/missing-constants/missing-constants.rb
