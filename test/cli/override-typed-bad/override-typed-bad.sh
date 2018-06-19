#!/bin/bash
main/sorbet --typed-override=test/cli/override-typed-bad/override-typed-bad.yaml test/cli/override-typed-bad/override-typed-bad.rb 2>&1
main/sorbet --typed-override=test/cli/override-typed-bad/override-typed-bad.yaml -e "1" 2>&1
