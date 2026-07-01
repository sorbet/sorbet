#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message --error-url-base "https://example.com/errors/?error=" test/cli/error-url/error-url.rb 2>&1
main/sorbet --silence-dev-message --error-url-base "https://example.com/error/" test/cli/error-url/error-url.rb 2>&1
main/sorbet --silence-dev-message --error-url-base "https://example.com/error#" test/cli/error-url/error-url.rb 2>&1
