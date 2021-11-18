# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

def no_block
  p ["yo","yo","yah","ah","oh","yo","yah","oh","oh","eh"].uniq
  p [1, 2, 3, 2, 3, 4, 1, 1, 5, 8, 6].uniq
end

# INITIAL-LABEL: define internal i64 @"func_Object#8no_block"
# INITIAL: call i64 @sorbet_rb_array_uniq(
# INITIAL: call i64 @sorbet_rb_array_uniq(
# INITIAL{LITERAL}: }

no_block

def with_block
  p ["yo","yo","yah","ah","oh","yo","yah","oh","oh","eh"].uniq { |x| x.reverse }
  p ["yo","yo","yah","ah","oh","yo","yah","oh","oh","eh"].uniq { |x| x.length }
  p [1, 2, 3, 2, 3, 4, 1, 1, 5, 8, 6].uniq { |x| x%2 }
  p [1, 2, 3, 4].uniq { break "ope" }
end

# INITIAL-LABEL: define internal i64 @"func_Object#10with_block"
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_array_uniq_withBlock
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_array_uniq_withBlock
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_array_uniq_withBlock
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock(i64 (i64)* @forward_sorbet_rb_array_uniq_withBlock
# INITIAL{LITERAL}: }

with_block
