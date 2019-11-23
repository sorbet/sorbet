#!/bin/bash

main/sorbet --silence-dev-message --no-stdlib --stop-after semantic-extension -p symbol-table test/cli/stop-after-semantic-extension/stop-after-semantic-extension.rb 2>&1
