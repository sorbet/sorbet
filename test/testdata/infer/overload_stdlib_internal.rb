# typed: __STDLIB_INTERNAL
extend T::Sig

sig { params(x: String).returns(String) }
sig { params(x: Integer).returns(Integer) }
def foo(x) # error: Refusing to typecheck `Object#foo` against an overloaded signature
  raise
end

res = foo('')
T.reveal_type(res) # error: `String`

res = foo(0)
T.reveal_type(res) # error: `Integer`
