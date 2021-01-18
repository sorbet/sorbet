# typed: true

extend T::Sig

sig {params(x: {foo: Integer}).void}
def test2(x)
  x[:foo] = 1
  x[:foo] = '' # error: Expected `Integer` but found `String("")` for key `Symbol(:foo)`

  # Likewise, neither of these report errors, for backwards compatibility
  # We should work to make this stricter.
  x[:bar] = 1
  x[:bar] = ''
end

sig {params(x: {foo: Integer, 'bar' => Float}).void}
def test3(x)
  x[:foo] = '' # error: Expected `Integer` but found `String("")` for key `Symbol(:foo)`
  T.reveal_type(x[:foo]) # error: Revealed type: `T.untyped`

  # This won't error until we change the `underlying` on Shape types to be stricter
  x[true] = nil
end
