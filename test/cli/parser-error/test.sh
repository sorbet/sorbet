#!/bin/bash

main/sorbet --silence-dev-message test/cli/parser-error/parser-error-1.rb 2>&1
main/sorbet --silence-dev-message test/cli/parser-error/parser-error-2.rb 2>&1
main/sorbet --silence-dev-message test/cli/parser-error/parser-error-3.rb 2>&1
main/sorbet --silence-dev-message test/cli/parser-error/parser-error-4.rb 2>&1
