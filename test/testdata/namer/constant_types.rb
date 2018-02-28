# @typed
module Constants
  A = 1
  B = "hi"
  C = :hi
  D = 1.0
  E = T.let(some_value, T::Array[Integer])
  F = T.assert_type!(some_value, T::Array[String]) # error: Use T.let()
end
