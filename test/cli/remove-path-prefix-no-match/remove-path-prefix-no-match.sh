#!/bin/bash
main/sorbet --silence-dev-message -d test/cli/remove-path-prefix-no-match --remove-path-prefix test/unknown/path 2>&1
