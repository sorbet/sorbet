# frozen_string_literal: true
# typed: true
# compiled: true

x = T.let([1,2,3,4], T::Array[Integer])

# These don't typecheck given Sorbet's RBI definitions even though they work in
# normal Ruby.
# x[1..2] = 0
# puts x              # [1,0,4]
# x[1..2] = [5,5,5]
# puts x              # [1,5,5,5]
# x[0,2] = 0
# puts x              # [0,5,5]
# x[0,3] = [1,2,3,4]
# puts x              # [1,2,3,4]

x[1] = 0
puts x

x[2] = 1
puts x

x[10] = 1
puts x
