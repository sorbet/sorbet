# typed: strict

T.let(0, Integer) { puts }
#                ^^^^^^^^^ error: Method `T.let` does not take a block

X = T.let(0, Integer) { puts }
#                    ^^^^^^^^^ error: Method `T.let` does not take a block
#   ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Constants must have type annotations with `T.let`
