# typed: strict

A = _ # error: Constants must have type annotations with T.let() when specifying '# typed: strict'

B = T.let(_, T.untyped)

C = T.let(1, Integer)

D = T.type_alias(Integer)
