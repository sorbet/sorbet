# typed: true
extend T::Sig

sig {params(x: T.untyped).void}
def foo(x)
  x.
  # ^ completion: (nothing)
end # error: unexpected token
