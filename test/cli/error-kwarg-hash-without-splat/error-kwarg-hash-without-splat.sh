#!/bin/bash

main/sorbet --silence-dev-message --error-kwarg-hash-without-splat test/cli/error-kwarg-hash-without-splat/goodsplat.rb 2>&1
main/sorbet --silence-dev-message test/cli/error-kwarg-hash-without-splat/goodsplat.rb 2>&1
main/sorbet --silence-dev-message --error-kwarg-hash-without-splat test/cli/error-kwarg-hash-without-splat/badsplat.rb 2>&1
main/sorbet --silence-dev-message test/cli/error-kwarg-hash-without-splat/badsplat.rb 2>&1
