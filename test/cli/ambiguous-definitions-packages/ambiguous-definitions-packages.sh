#!/bin/bash

set -e

cd test/cli/ambiguous-definitions-packages || exit 1

../../../main/sorbet --silence-dev-message --stripe-mode --stripe-packages . 2>&1


