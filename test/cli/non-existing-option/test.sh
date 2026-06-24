#!/bin/bash
set -euo pipefail

main/sorbet --long-option 2>&1
main/sorbet -short-option 2>&1
