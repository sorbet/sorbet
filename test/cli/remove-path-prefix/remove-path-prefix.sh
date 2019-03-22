#!/bin/bash
main/sorbet --silence-dev-message test/cli/remove-path-prefix --remove-path-prefix test/cli/remove-path-prefix/ 2>&1
