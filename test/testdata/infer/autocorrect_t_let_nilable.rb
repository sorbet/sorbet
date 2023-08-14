# typed: true

x = T.let(5, T.nilable(Integer))
  T.let(x, Integer)
# ^^^^^^^^^^^^^^^^^ error: Argument does not have asserted type `Integer`
