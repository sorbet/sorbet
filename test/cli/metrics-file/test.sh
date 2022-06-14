#!/bin/bash

main/sorbet --silence-dev-message -e 'class Foo; end' --metrics-file=metrics.json 2>&1
grep status metrics.json

echo ------------------------------

# Sorbet does not crash if the metrics directory does not exist
main/sorbet --silence-dev-message -e 'class Foo; end' --metrics-file=foo/bar/metrics2.json 2>&1

echo ------------------------------

# Sorbet produce a clean error if the metrics path is not a directory
touch baz
main/sorbet --silence-dev-message -e 'class Foo; end' --metrics-file=baz/metrics3.json 2>&1

echo ------------------------------

# Sorbet will not count errors that are created but end up being
# ignored by other branches, e.g. when a type error is found in one
# branch of a T.all but ends up being made irrelevant by the other
main/sorbet --silence-dev-message --metrics-file=metrics4.json test/cli/metrics-file/with-error-branching.rb 2>&1
grep -A1 "\"ruby_typer.unknown..error.total\"" metrics4.json || echo "No error metrics reported."

grep -A1 "\"ruby_typer.unknown..error.total\"" metrics5.json || echo "No error metrics reported."
