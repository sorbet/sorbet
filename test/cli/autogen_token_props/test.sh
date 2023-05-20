#!/bin/bash
set -eu
cd test/cli/autogen_token_props || exit 1

../../../main/sorbet --silence-dev-message -p token-prop-analysis --stop-after=namer . 2>&1

