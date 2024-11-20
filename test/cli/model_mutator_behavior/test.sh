#!/bin/bash

set -euo pipefail

main/sorbet --silence-dev-message --uniquely-defined-behavior test/cli/model_mutator_behavior/*.rb 2>&1
