# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

result = [1, 2, 3].map do |x|
  puts "-- #{x} --"
  x
end

# INITIAL-LABEL: define internal i64 @"func_<root>.<static-init>
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock(i64 (i64)* @forward_sorbet_rb_array_collect
# INITIAL{LITERAL}: }

p result
