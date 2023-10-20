# typed: true

def bad_method(x) # error: Refusing to typecheck `Object#bad_method` against an overloaded signature
  T.reveal_type(x)
end

res = good_method(0)
T.reveal_type(res) # error: `Integer`
res = good_method('')
T.reveal_type(res) # error: `String`
res = good_method(:foo)
#                 ^^^^ error: Expected `Integer` but found `Symbol(:foo)` for argument `x`

res = bad_method(0)
T.reveal_type(res) # error: `Integer`
res = bad_method('')
T.reveal_type(res) # error: `String`
res = bad_method(:foo)
#                ^^^^ error: Expected `Integer` but found `Symbol(:foo)` for argument `x`
