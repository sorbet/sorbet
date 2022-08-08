#!/bin/bash

main/sorbet --censor-for-snapshot-tests --silence-dev-message test/cli/folder-input/folder-input.rb test/cli/folder-input/input 2>&1
