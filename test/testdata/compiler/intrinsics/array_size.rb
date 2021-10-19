# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

sig {params(ary: T::Array[Integer]).returns(Integer)}
def ary_length(ary)
  ary.length
end

# INITIAL-LABEL: "func_Object#10ary_length"
# INITIAL: call i64 @sorbet_rb_array_len
# INITIAL{LITERAL}: }

sig {params(ary: T::Array[Integer]).returns(Integer)}
def ary_size(ary)
  ary.size
end

# INITIAL-LABEL: "func_Object#8ary_size"
# INITIAL: call i64 @sorbet_rb_array_len
# INITIAL{LITERAL}: }

p ary_length([1, 2, 3])
p ary_size([1, 2, 3])
p ary_length([])
p ary_size([])
