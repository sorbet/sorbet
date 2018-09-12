# typed: true

A = 3
B = 4

T.any(A,B) # error: Unsupported usage of literal type
T.all(A,B) # error: Unsupported usage of literal type
T::Array[A] # error: Unsupported usage of literal type
