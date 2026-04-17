#!/bin/bash
set -euo pipefail

cd gems/sorbet-runtime
# Inherits arguments from sorbet/config
../../main/sorbet
