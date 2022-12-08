# typed: true

T.reveal_type(A) # error: `T.untyped`

T.reveal_type(B) # error: `T.untyped`

T.reveal_type(C1) # error: `T.untyped`
T.reveal_type(C2) # error: `NewIsSpecific`
T.reveal_type(C3) # error: `T.untyped`

