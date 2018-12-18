#!/bin/bash
main/sorbet --silence-dev-message -e 'class Foo; end' -p symbol-table
