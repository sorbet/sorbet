# typed: true

A = nil
B = nil

case nil
in A
in A | B
in A, B
in A => x
  T.reveal_type(x) # error: Revealed type: `T.untyped`
in A | B => x
  T.reveal_type(x) # error: Revealed type: `T.untyped`
in A => x, B => y
  T.reveal_type(x) # error: Revealed type: `T.untyped`
  T.reveal_type(y) # error: Revealed type: `T.untyped`
end
