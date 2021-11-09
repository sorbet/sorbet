# typed: true

A = 3
B = 4

T.any(A,B)
    # ^ error: Unsupported usage of bare type
      # ^ error: Unsupported usage of bare type
T.any(A, Integer) # error: Unsupported usage of bare type
T.any(Integer, A) # error: Unsupported usage of bare type
T.all(A,B) # error: Unsupported usage of bare type
      # ^ error: Unsupported usage of bare type
T::Array[A] # error: Unsupported usage of bare type
