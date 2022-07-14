#!/bin/bash

main/sorbet --silence-dev-message --ignore usually_ignored/ --unignore /subfolder --ignore foo.rb --unignore foo.rb --dir test/cli/unignore 2>&1
