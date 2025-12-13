# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

result = [1, 2, 3, 4, 5, 6].reject do |x|
  x.even?
end

puts result

puts ([1, 2, 3, 4, 5, 6].reject do |x|
        break "ope"
      end)

# INITIAL-LABEL: define internal i64 @"func_<root>.13<static-init>
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_array_reject_withBlock
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock(i64 (i64)* @forward_sorbet_rb_array_reject_withBlock
# INITIAL{LITERAL}: }

def foo(&blk)
  result = [1, 2, 3, 4, 5, 6].reject(&blk)
  puts result
end

foo do |x|
  x.even?
end
