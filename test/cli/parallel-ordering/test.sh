#!/bin/bash

dir="test/cli/parallel-ordering"
main/sorbet  --silence-dev-message --max-threads=0 $dir/1.rb $dir/2.rb $dir/3.rb 2>&1
