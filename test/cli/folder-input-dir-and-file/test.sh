#!/bin/bash

main/sorbet --censor-for-snapshot-tests --silence-dev-message --file test/cli/folder-input-dir-and-file/foo.rb --dir test/cli/folder-input-dir-and-file/input 2>&1
