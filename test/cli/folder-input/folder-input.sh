#!/bin/bash

main/sorbet --silence-dev-message test/cli/folder-input/foo.rb -d test/cli/folder-input/input 2>&1
