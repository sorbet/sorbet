# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

h = {a: 1, b: 2, c: :sym}

# single-arg blocks receive [key, value] arrays
result = h.any? do |a|
  a[1] == 2
end

p result

result = h.any? do |a|
  a[0] == 3
end

p result

# INITIAL-COUNT-2: call i64 @sorbet_inlineIntrinsicEnv_apply(i64 %0, i64 (i64, i64, i32, i64*, i64 (i64, i64, i32, i64*, i64)*, %struct.rb_captured_block*, i64, i32)* @sorbet_rb_hash_any_withBlock
