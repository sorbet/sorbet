# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

def test_array_concat
  p ([1, 2, 3].concat)
  p ([1, 2, 3].concat([]))
  p ([1, 2, 3].concat([],[]))
  p ([1, 2, 3].concat([],[3,2,1]))
  p ([1, 2, 3].concat([1,2,3],[4,5,6],[7,8,9,10],["greetings"]))
  p ([].concat([1,2,3],[4,5,6],[7,8,9,10],["greetings"]))
end

test_array_concat

# INITIAL-LABEL: define internal i64 @"func_Object#17test_array_concat"
# INITIAL: call i64 @sorbet_int_rb_ary_concat_multi(
# INITIAL: call i64 @sorbet_int_rb_ary_concat_multi(
# INITIAL: call i64 @sorbet_int_rb_ary_concat_multi(
# INITIAL: call i64 @sorbet_int_rb_ary_concat_multi(
# INITIAL: call i64 @sorbet_int_rb_ary_concat_multi(
# INITIAL: call i64 @sorbet_int_rb_ary_concat_multi(
# INITIAL{LITERAL}: }
