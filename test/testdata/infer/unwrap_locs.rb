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

F = 3.0
G = 4.0

T.any(F,G)
    # ^ error: Unexpected bare `Float` value found in type position
      # ^ error: Unexpected bare `Float` value found in type position
T.any(F, Float) # error: Unexpected bare `Float` value found in type position
T.any(Float, F) # error: Unexpected bare `Float` value found in type position
T.all(F,G) # error: Unexpected bare `Float` value found in type position
      # ^ error: Unexpected bare `Float` value found in type position
T::Array[F] # error: Unexpected bare `Float` value found in type position

S = "something"
R = "else"

T.any(S,R)
    # ^ error: Unexpected bare `String` value found in type position
      # ^ error: Unexpected bare `String` value found in type position
T.any(S, String) # error: Unexpected bare `String` value found in type position
T.any(String, S) # error: Unexpected bare `String` value found in type position
T.all(S,R) # error: Unexpected bare `String` value found in type position
      # ^ error: Unexpected bare `String` value found in type position
T::Array[S] # error: Unexpected bare `String` value found in type position

Y = :even
M = :more

T.any(Y,M)
    # ^ error: Unexpected bare `Symbol` value found in type position
      # ^ error: Unexpected bare `Symbol` value found in type position
T.any(Y, Symbol) # error: Unexpected bare `Symbol` value found in type position
T.any(Symbol, Y) # error: Unexpected bare `Symbol` value found in type position
T.all(Y,M) # error: Unexpected bare `Symbol` value found in type position
      # ^ error: Unexpected bare `Symbol` value found in type position
T::Array[Y] # error: Unexpected bare `Symbol` value found in type position
