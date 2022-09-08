#!/bin/bash

file --brief --dereference test/cli/windows-line-endings/windows-line-endings.rb

main/sorbet --silence-dev-message test/cli/windows-line-endings/windows-line-endings.rb 2>&1
