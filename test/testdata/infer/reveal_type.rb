# typed: true
T.reveal_type(0) # error: Revealed type: `Integer(0)`
T.reveal_type(T.unsafe(nil)) # error: Revealed type: `T.untyped`
T.reveal_type # error: Wrong number of arguments provided

T.assert_type!(T.reveal_type(0), Integer) # error: Revealed type:
