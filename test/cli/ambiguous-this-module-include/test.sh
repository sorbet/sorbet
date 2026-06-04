#!/bin/bash

set -e

main/sorbet --silence-dev-message test/cli/ambiguous-this-module-include/ 2>&1

