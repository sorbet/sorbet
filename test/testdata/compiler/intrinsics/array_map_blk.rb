# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

# Break is only used in the second call to `Array#map`, so we should see that the first version uses the `noBreak`
# variant of `sorbet_callIntrinsicInlineBlock`, and the second does not.

# INITIAL-LABEL: define internal i64 @"func_<root>.13<static-init>
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_array_collect
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock(i64 (i64)* @forward_sorbet_rb_array_collect
# INITIAL{LITERAL}: }


# Additionally, in the optimized output, we should see exactly one call to `sorbet_rb_iterate` (from the use of
# `sorbet_callIntrinsicInlineBlock`) in the Init function, as the root static init will be inlined there.

# OPT-LABEL: define void @Init_array_map_blk()
# OPT: call i64 @sorbet_rb_iterate(
# OPT-NOT: call i64 @sorbet_rb_iterate(
# OPT{LITERAL}: }

result = [1, 2, 3].map do |x|
  puts "-- #{x} --"
  x
end

p result

result = [1, 2, 3, 4, 5, 6, 7, 8].map do |x|
  puts "-- #{x} --"
  break :finished if x == 6
end

p result

