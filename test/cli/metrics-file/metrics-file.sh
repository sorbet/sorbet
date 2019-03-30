#!/bin/bash
main/sorbet --silence-dev-message -e 'class Foo; end' --metrics-file=metrics.json

grep status metrics.json
