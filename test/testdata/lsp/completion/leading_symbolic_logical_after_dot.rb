# typed: true

extend T::Sig

module M
  def bar; end
  def foo; end
end

sig {params(x: M, y: T::Boolean).void}
def symbolic_and(x, y)
  x.
  # ^ completion: bar, foo, ...
  && y # error: unexpected token
end

sig {params(x: M, y: T::Boolean).void}
def symbolic_or(x, y)
  x.
  # ^ completion: bar, foo, ...
  || y # error: unexpected token
end
