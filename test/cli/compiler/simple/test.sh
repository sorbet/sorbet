#!/usr/bin/env bash

set -euo pipefail

sorbet -e 'puts "Hello, world"'
ruby -e 'puts "Hello, world"'
