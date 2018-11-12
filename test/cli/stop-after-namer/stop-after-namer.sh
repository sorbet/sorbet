#!/bin/bash

# Our code is factored in a weird way, so it's easy for the condition to check
# for --stop-after namer to get out of sync.
main/sorbet --no-stdlib --stop-after namer -p symbol-table test/cli/stop-after-namer/stop-after-namer.rb 2>&1
