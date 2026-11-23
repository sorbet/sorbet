# typed: true

A = nil
B = nil

case T.unsafe(nil)
in A
in A | B
in A, B
in A => x
  T.reveal_type(x) # error: Revealed type: `NilClass`
in A | B => x
  T.reveal_type(x) # error: Revealed type: `NilClass`
in A => x, B => y
  T.reveal_type(x) # error: Revealed type: `NilClass`
  T.reveal_type(y) # error: Revealed type: `NilClass`
end
