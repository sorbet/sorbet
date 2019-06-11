# typed: true

x = T.unsafe(0)
case x
when String
  T.reveal_type(x) # error: Revealed type: `String`
when Integer
  T.reveal_type(x) # error: Revealed type: `Integer`
else
  T.reveal_type(x) # error: Revealed type: `T.untyped`
end
