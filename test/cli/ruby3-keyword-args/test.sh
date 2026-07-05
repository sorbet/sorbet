#!/bin/bash

main/sorbet --silence-dev-message --experimental-ruby3-keyword-args test/cli/ruby3-keyword-args/goodsplat.rb 2>&1
main/sorbet --silence-dev-message test/cli/ruby3-keyword-args/goodsplat.rb 2>&1
main/sorbet --silence-dev-message --experimental-ruby3-keyword-args test/cli/ruby3-keyword-args/badsplat.rb 2>&1
main/sorbet --silence-dev-message test/cli/ruby3-keyword-args/badsplat.rb 2>&1
