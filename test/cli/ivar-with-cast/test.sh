#!/bin/bash

set -e

main/sorbet --silence-dev-message test/cli/ivar-with-cast/ivar-with-cast.rb 2>&1
