#!/bin/bash

set -e

cd test/cli/ambiguous-definitions-packages || exit 1

../../../main/sorbet --silence-dev-message --report-ambiguous-definitions --stripe-packages . 2>&1


