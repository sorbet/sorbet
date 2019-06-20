#!/bin/bash

main/sorbet --silence-dev-message --file test/cli/folder-input-dir-and-file/foo.rb --dir test/cli/folder-input-dir-and-file/input 2>&1
