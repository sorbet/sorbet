#!/bin/bash
main/sorbet --censor-for-snapshot-tests --silence-dev-message --dir test/cli/remove-path-prefix --remove-path-prefix test/cli/remove-path-prefix/ 2>&1
