#!/usr/bin/env bash

cd test/cli/package-non-prefix-collision || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1
