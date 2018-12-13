# typed: true

A = 3
B = 4

T.any(A,B) # error: MULTI
T.any(A, Integer) # error: Unsupported usage of literal type
T.any(Integer, A) # error: Unsupported usage of literal type
T.all(A,B) # error: MULTI
T::Array[A] # error: Unsupported usage of literal type
