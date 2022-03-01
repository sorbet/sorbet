#!/bin/bash
main/sorbet --silence-dev-message test/cli/forgot-typed/forgot-typed.rb 2>&1
main/sorbet --silence-dev-message test/cli/forgot-typed/permit-dsl-sig.rb 2>&1
