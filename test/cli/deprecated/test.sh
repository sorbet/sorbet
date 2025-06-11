#!/bin/bash
exec 2>&1
D=test/cli/deprecated
main/sorbet --silence-dev-message $D/t1.rb
main/sorbet --silence-dev-message $D/t2.rb
