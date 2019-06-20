#!/bin/bash

main/sorbet --silence-dev-message -e 'class Foo; end' --metrics-file=metrics.json 2>&1
grep status metrics.json

echo ------------------------------

# Sorbet does not crash if the metrics directory does not exist
main/sorbet --silence-dev-message -e 'class Foo; end' --metrics-file=foo/bar/metrics2.json 2>&1
grep status foo/bar/metrics2.json

echo ------------------------------

# Sorbet produce a clean error if the metrics path is not a directory
touch baz
main/sorbet --silence-dev-message -e 'class Foo; end' --metrics-file=baz/metrics3.json 2>&1
