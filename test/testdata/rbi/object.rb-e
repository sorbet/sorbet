# typed: true

obj = T.let("foo", String)

# Object#then, with a block
T.reveal_type(obj.then(&:to_i)) # error: Revealed type: `T.untyped`
