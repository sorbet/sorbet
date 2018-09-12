# typed: true
module Constants
  A = 1
  B = "hi"
  C = :hi
  D = 1.0
  E = T.let([], T::Array[Integer])
  F = T.assert_type!([], T::Array[String]) # error: Use T.let()
end
