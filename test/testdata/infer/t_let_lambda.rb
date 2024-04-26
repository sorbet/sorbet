# typed: true

f = T.let(
  -> (x) do
    T.reveal_type(x)
  end,
  T.proc.params(x: Integer).returns(Integer)
)
T.reveal_type(f)

f = T.let(
  -> (x) do
    T.reveal_type(x)
    x.to_s # error: Expected `Integer` but found `String` for block result type
  end,
  T.proc.params(x: Integer).returns(Integer)
)
T.reveal_type(f)

# Should we error if this is not a proc type?
# (What would have happened if you put `blk: Integer` in a sig somewhere?)
f = T.let(
  -> (x) do
    T.reveal_type(x)
  end,
  Integer
)
T.reveal_type(f)
