#!/bin/bash
script="$1"
expect="$2"

diff -u <("$script" | sed -e 's,\(rbi/stdlib.rbi:\)[0-9]*,\1__LINE__,') "$expect"
