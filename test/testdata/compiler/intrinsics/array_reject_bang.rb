# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

def test_blk
  arr = [1, 2, 3, 4, 5]
  arr.reject! { |x| x%2 == 0 }

  p arr
  p arr.length
end

test_blk

# INITIAL-LABEL: define internal i64 @"func_Object#8test_blk"(
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_array_reject_bang_withBlock
# INITIAL{LITERAL}: }

def test_blk_raise
  arr = [1, 2, 3, 4, 5]
  begin
    arr.reject! do |x|
      if x > 3
        raise "whoops"
      end
      x%2 == 0
    end
  rescue => e
    puts e
  end

  p arr
  p arr.length
end

test_blk_raise

# INITIAL-LABEL: define internal i64 @"func_Object#14test_blk_raise"(
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_array_reject_bang_withBlock
# INITIAL{LITERAL}: }

def test_blk_break
  arr = [1, 2, 3, 4, 5]
  arr.reject! do |x|
    if x > 3
      break
    end
    x%2 == 0
  end

  p arr
  p arr.length
end

test_blk_break

# INITIAL-LABEL: define internal i64 @"func_Object#14test_blk_break"(
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock(i64 (i64)* @forward_sorbet_rb_array_reject_bang_withBlock
# INITIAL{LITERAL}: }

def test_no_blk
  arr = [1, 2, 3, 4, 5]
  e = arr.reject!

  p arr
  p arr.length

  e.each { |x| x%2 == 0 }

  p arr
  p arr.length
end

test_no_blk

# INITIAL-LABEL: define internal i64 @"func_Object#11test_no_blk"(
# INITIAL: call i64 @sorbet_rb_array_reject_bang(
# INITIAL{LITERAL}: }
