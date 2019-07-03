#!/bin/bash
exec 2>&1
D=test/cli/rbi-overrides
main/sorbet --silence-dev-message $D/t1.rbi $D/t1.rb
main/sorbet --silence-dev-message $D/t2.rbi $D/t2.rb
main/sorbet --silence-dev-message $D/t3.rbi $D/t3.rb
main/sorbet --silence-dev-message $D/t4.rbi $D/t4.rb
