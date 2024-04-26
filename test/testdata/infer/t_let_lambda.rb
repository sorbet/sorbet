# typed: true

f = T.let(
  -> (x) do
    T.reveal_type(x) # error: `Integer`
  end,
  T.proc.params(x: Integer).returns(Integer)
)
T.reveal_type(f) # error: `T.proc.params(arg0: Integer).returns(Integer)`

f = T.let(
  -> (x) do
    T.reveal_type(x)
    x.to_s # error: Expected `Integer` but found `String` for block result type
  end,
  T.proc.params(x: Integer).returns(Integer)
)
T.reveal_type(f) # error: `T.proc.params(arg0: Integer).returns(Integer)`

f = T.let(
  -> (x) do
    T.reveal_type(x) # error: `T.untyped`
  end,
  Integer
# ^^^^^^^ error: Lambda type annotation must be either `Proc` or a `T.proc` type (and possibly nilable)
)
T.reveal_type(f) # error: `Integer`

f = T.cast(
  -> (x) do
    T.reveal_type(x) # error: `T.untyped`
  end,
  Integer
# ^^^^^^^ error: Lambda type annotation must be either `Proc` or a `T.proc` type (and possibly nilable)
)
T.reveal_type(f) # error: `Integer`
