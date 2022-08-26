# typed: true

X = T.let('', T.nilable(String))
T.reveal_type(X) # error: `T.nilable(String)`
X = 0
#   ^ error: Expected `T.nilable(String)` but found `Integer(0)` for field
T.reveal_type(X) # error: `T.nilable(String)`
