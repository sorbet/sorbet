# typed: true

extend T::Sig

def test1
  x = {foo: 1}

  T.reveal_type(x[:foo]) # error: Revealed type: `Integer(1)`

  # This is untyped for backwards compatibility with the original shape implementation.
  # We didn't want to introduce so many errors all at once, because the migration effort was huge.
  T.reveal_type(x[:bar]) # error: Revealed type: `T.untyped`

  # This doesn't raise an error either (String access, not Symbol access)
  T.reveal_type(x['bar']) # error: Revealed type: `T.untyped`
end
