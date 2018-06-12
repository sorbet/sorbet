#!/bin/bash
script="$1"
expect="$2"

diff "$expect" -u <("$script")
