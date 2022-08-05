#!/bin/bash
main/sorbet --censor-for-snapshot-tests --silence-dev-message --dir test/cli/remove-path-prefix-no-match --remove-path-prefix test/unknown/path 2>&1
