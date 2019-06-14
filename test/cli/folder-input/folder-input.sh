#!/bin/bash

main/sorbet --silence-dev-message test/cli/folder-input/foo.rb --dir test/cli/folder-input/input 2>&1
