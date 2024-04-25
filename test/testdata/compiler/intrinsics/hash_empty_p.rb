# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

def test_empty_p
  h = {}
  p (h.empty?)

  h = {x: 33}
  p (h.empty?)
end

# INITIAL-LABEL: define internal i64 @"func_Object#12test_empty_p"
# INITIAL: call i64 @sorbet_rb_hash_empty_p(
# INITIAL: call i64 @sorbet_rb_hash_empty_p(
# INITIAL{LITERAL}: }
