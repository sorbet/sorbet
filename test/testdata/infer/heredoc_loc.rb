# typed: true

extend T::Sig

sig { params(x: Integer).void }
def foo(x); end

foo(<<~MSG) # error: Expected `Integer` but found `String("foo\n")` for argument `x`
  foo
MSG
