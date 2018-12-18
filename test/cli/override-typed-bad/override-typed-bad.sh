#!/bin/bash
main/sorbet --silence-dev-message --typed-override=test/cli/override-typed-bad/override-typed-bad.yaml test/cli/override-typed-bad/override-typed-bad.rb 2>&1
