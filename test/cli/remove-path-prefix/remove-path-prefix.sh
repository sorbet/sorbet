#!/bin/bash
main/sorbet --silence-dev-message -d test/cli/remove-path-prefix --remove-path-prefix test/cli/remove-path-prefix/ 2>&1
