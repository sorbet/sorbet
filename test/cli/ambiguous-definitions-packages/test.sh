#!/bin/bash

set -e

cd test/cli/ambiguous-definitions-packages || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1


