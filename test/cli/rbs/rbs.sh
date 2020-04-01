#!/bin/bash

dir="test/cli/rbs"

main/sorbet --silence-dev-message --experimental-rbs $dir/sorbet/rbs/a.rbs $dir/ok.rb 2>&1

echo -e "\n------------------------------------------------------------------\n"

main/sorbet --silence-dev-message --experimental-rbs $dir/sorbet/rbs/a.rbs $dir/ko.rb 2>&1

echo -e "\n------------------------------------------------------------------\n"

main/sorbet --silence-dev-message --experimental-rbs $dir/sorbet/rbs/error.rbs 2>&1
