# typed: strong
extend T::Sig

sig {params(x: Integer).void}
def takes_integer(x)
end

sig {params(x: Integer).void}
def takes_integer_kw(x:)
end

sig {params(y: T.untyped).void}
def example(y)
  takes_integer(y) # error: Argument passed to parameter `x` is `T.untyped`
  takes_integer_kw(y) # error: Positional argument being interpreted as keyword argument is `T.untyped`
  takes_integer_kw(x: y) # error: Argument passed to parameter `x` is `T.untyped`
end
