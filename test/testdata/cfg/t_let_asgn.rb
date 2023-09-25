# typed: true
extend T::Sig

sig {returns(Integer)}
def foo
  z =
    if T.unsafe(nil)
      0
    else
      T.let(0, Integer)
    end
  T.reveal_type(z) # error: `Integer`

  if T.unsafe(nil)
    T.let('', String)
  else
    "a"
  end
end # error: Expected `Integer` but found `String` for method result type
