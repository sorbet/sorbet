# typed: true

A = 3
B = 4

T.any(A,B)
    # ^ error: Unsupported usage of literal type
      # ^ error: Unsupported usage of literal type
T.any(A, Integer) # error: Unsupported usage of literal type
T.any(Integer, A) # error: Unsupported usage of literal type
T.all(A,B) # error: Unsupported usage of literal type
      # ^ error: Unsupported usage of literal type
T::Array[A] # error: Unsupported usage of literal type
