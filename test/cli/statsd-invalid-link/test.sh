#!/bin/bash
# checking exception is raised when host is invalid
main/sorbet --silence-dev-message -e 'class Foo; end' --statsd-host="f4K#l1N&.com" --statsd-prefix=foo.bar 2> >(grep --color=never "statsd initialization failed")
