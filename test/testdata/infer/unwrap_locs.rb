# typed: true

A = 3
B = 4

T.any(A,B)
    # ^ error: Unexpected bare `Integer` value found in type position
      # ^ error: Unexpected bare `Integer` value found in type position
T.any(A, Integer) # error: Unexpected bare `Integer` value found in type position
T.any(Integer, A) # error: Unexpected bare `Integer` value found in type position
T.all(A,B) # error: Unexpected bare `Integer` value found in type position
      # ^ error: Unexpected bare `Integer` value found in type position
T::Array[A] # error: Unexpected bare `Integer` value found in type position
