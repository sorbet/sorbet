# typed: strict
A = 1

K = B
#   ^ error: Unable to resolve constant `B`
#   ^ error: Constants must have type annotations with `T.let`
