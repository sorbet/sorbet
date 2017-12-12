# @typed
module Constants
  A = 1
  B = "hi"
  C = :hi
  D = 1.0
  E = Opus::Types.cast(some_value, Opus::Types.array_of(Integer))
  F = Opus::Types.assert_type!(some_value, Opus::Types.array_of(String)) # error: Use cast()
end
