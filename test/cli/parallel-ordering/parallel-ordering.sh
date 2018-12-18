#!/bin/bash

dir="test/cli/parallel-ordering"
main/sorbet --silence-dev-message $dir/1.rb $dir/2.rb $dir/3.rb 2>&1
