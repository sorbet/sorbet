# typed: true

def foo(x) # error: Refusing to typecheck `Object#foo` against an overloaded signature
  T.reveal_type(x)
end
