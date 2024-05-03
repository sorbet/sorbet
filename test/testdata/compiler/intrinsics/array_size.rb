# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(ary: T::Array[Integer]).returns(Integer)}
def ary_length(ary)
  ary.length
end


sig {params(ary: T::Array[Integer]).returns(Integer)}
def ary_size(ary)
  ary.size
end


p ary_length([1, 2, 3])
p ary_size([1, 2, 3])
p ary_length([])
p ary_size([])
