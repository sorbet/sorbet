# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

result = [1, 2, 3]

result.map! do |x|
  puts "-- #{x} --"
  x + 1
end

# INITIAL-LABEL: define internal i64 @"func_<root>.13<static-init>
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_array_collect_bang
# INITIAL{LITERAL}: }

p result

array = [1, 2, 3, 4, 5, 6, 7, 8]

result = array.map! do |x|
  break :finished if x == 6
  puts "-- #{x} --"
  x + 1
end

p result
p array
