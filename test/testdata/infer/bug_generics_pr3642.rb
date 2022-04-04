# frozen_string_literal: true
# typed: strict
extend T::Sig

sig do
  type_parameters(:U)
    .params(
      f: T.proc.params(x: T.type_parameter(:U)).void,
      x: T.type_parameter(:U),
    )
    .returns(T.type_parameter(:U))
end
def f_x(f, x)
  f.call(x)
  x
end

sig do
  type_parameters(:U)
    .params(
      x: T.type_parameter(:U),
      f: T.proc.params(x: T.type_parameter(:U)).void,
    )
    .returns(T.type_parameter(:U))
end
def x_f(x, f)
  f.call(x)
  x
end

f = T.let(
  proc {|x| nil},
  T.proc.params(x: T.any(String, Integer)).void
)

g = T.let(
  proc {|x| nil},
  T.proc.params(x: T.untyped).void
)

str_or_int = T.let(0, T.any(String, Integer))

res = f_x(f, 0)
T.reveal_type(res) # error: Revealed type: `T.any(String, Integer)`
res = x_f(0, f)
T.reveal_type(res) # error: Revealed type: `T.any(String, Integer)`

res = f_x(T.unsafe(f), 0)
T.reveal_type(res) # error: Revealed type: `T.untyped`
res = x_f(0, T.unsafe(f))
T.reveal_type(res) # error: Revealed type: `T.untyped`

res = f_x(f, T.unsafe(0))
T.reveal_type(res) # error: Revealed type: `T.any(String, Integer)`
res = x_f(T.unsafe(0), f)
T.reveal_type(res) # error: Revealed type: `T.any(String, Integer)`

# ---------------

res = f_x(g, str_or_int)
T.reveal_type(res) # error: Revealed type: `T.untyped`
res = x_f(str_or_int, g)
T.reveal_type(res) # error: Revealed type: `T.untyped`

res = f_x(T.unsafe(g), str_or_int)
T.reveal_type(res) # error: Revealed type: `T.untyped`
res = x_f(str_or_int, T.unsafe(g))
T.reveal_type(res) # error: Revealed type: `T.untyped`

res = f_x(g, T.unsafe(str_or_int))
T.reveal_type(res) # error: Revealed type: `T.untyped`
res = x_f(T.unsafe(str_or_int), g)
T.reveal_type(res) # error: Revealed type: `T.untyped`
