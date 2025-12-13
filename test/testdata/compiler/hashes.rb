# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

puts({a: 1, b: 2})

# INITIAL-LABEL: define internal i64 @"func_<root>.13<static-init>
# INITIAL: call i64 @sorbet_globalConstDupHash
# INITIAL{LITERAL}: }
