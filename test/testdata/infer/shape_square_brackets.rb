# typed: true

extend T::Sig

sig {params(x: {foo: Integer}).void}
def test2(x)
  x[:foo] = 1
  x[:foo] = '' # error:

  # Likewise, neither of these report errors, for backwards compatibility
  x[:bar] = 1
  x[:bar] = ''
end

sig {params(x: {foo: Integer, 'bar' => Float}).void}
def test3(x)
  # could make this better, e.g.: only allow setting to `Integer`
  x[:foo] = 0.0
  # ... especially because the type remains the same as before:
  T.reveal_type(x[:foo]) # error: Revealed type: `Integer`

  # This might end up being too restrictive.  This used to not error at all.
  x[true] = nil # error: Expected `T.any(Integer, Float)` but found `NilClass` for argument `arg1`
end
