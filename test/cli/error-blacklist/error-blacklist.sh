#!/bin/bash
main/sorbet --silence-dev-message test/cli/error-blacklist/error-blacklist.rb 2>&1
main/sorbet --silence-dev-message test/cli/error-blacklist/error-blacklist.rb --error-black-list=7002 2>&1
