#!/bin/bash
set -eux

diff -u test/dash-e-test.out <(main/ruby-typer -e 'class Foo; end' -p name-table)
