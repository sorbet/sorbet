#!/usr/bin/env bash
set -euo pipefail

main/sorbet --silence-dev-message --sorbet-packages . 2>&1
