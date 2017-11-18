#!/bin/bash

# re-record with
#  ./bazel-bin/main/ruby-typer test/end-to-end-test-input.rb > test/end-to-end-test-output 2>&1

diff -u test/end-to-end-test-output <(main/ruby-typer test/end-to-end-test-input.rb 2>&1) 
