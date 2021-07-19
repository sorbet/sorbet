#!/bin/bash

file --brief "$(realpath test/cli/windows-line-endings/windows-line-endings.rb)"

main/sorbet --silence-dev-message test/cli/windows-line-endings/windows-line-endings.rb 2>&1
