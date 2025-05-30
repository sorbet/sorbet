# typed: strict
extend T::Sig

T.let(0, Integer) { puts }
#                ^^^^^^^^^ error: Method `T.let` does not take a block

X = T.let(0, Integer) { puts }
#                    ^^^^^^^^^ error: Method `T.let` does not take a block
#   ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Constants must have type annotations with `T.let`


sig { params(x: T.nilable(Integer)).void }
def takes_nilable(x)
  T.must(x) { puts }
  #        ^^^^^^^^^ error: Method `T.must` does not take a block
end
