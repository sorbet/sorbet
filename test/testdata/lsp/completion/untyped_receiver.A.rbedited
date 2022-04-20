# typed: true
extend T::Sig

sig {params(x: T.untyped).void}
def foo(x)
  x.
  # ^ completion: (call site is T.untyped)
  # ^ apply-completion: [A] item: 0
end # error: unexpected token
