#!/bin/bash
main/sorbet --silence-dev-message -e 'class Foo; end' --statsd-host=127.0.0.1 # we're just checking if it even runs
