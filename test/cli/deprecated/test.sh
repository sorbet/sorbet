#!/bin/bash
exec 2>&1
D=test/cli/deprecated

main/sorbet --silence-dev-message $D/t1.rb --enable-experimental-rbs-comments --enable-deprecated
main/sorbet --silence-dev-message $D/t1.rb --enable-experimental-rbs-comments 

main/sorbet --silence-dev-message $D/t2.rb --enable-deprecated
main/sorbet --silence-dev-message $D/t2.rb
