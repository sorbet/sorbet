#!/bin/bash
main/sorbet --silence-dev-message -e '"Foo" * "Foo"' 2>&1
main/sorbet --silence-dev-message --no-stdlib -e '"Foo" * "Foo"' test/cli/no-stdlib/no-stdlib.rbi 2>&1
