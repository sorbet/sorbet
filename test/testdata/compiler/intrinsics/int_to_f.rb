# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: OPT

def test_int_to_f
  puts(-128.to_f)
  puts(-1.to_f)
  puts(0.to_f)
  puts(1.to_f)
  puts(127.to_f)

  # https://stackoverflow.com/questions/3793838/which-is-the-first-integer-that-an-ieee-754-float-is-incapable-of-representing-e
  puts(9_007_199_254_740_992.to_f.to_i)
end

# OPT-LABEL: define internal i64 @"func_Object#13test_int_to_f"
# OPT: tail call i64 @sorbet_int_to_f(
# OPT: tail call i64 @sorbet_int_to_f(
# OPT: tail call i64 @sorbet_int_to_f(
# OPT: tail call i64 @sorbet_int_to_f(
# OPT: tail call i64 @sorbet_int_to_f(
# OPT: tail call i64 @sorbet_int_to_f(
# OPT{LITERAL}: }

test_int_to_f
