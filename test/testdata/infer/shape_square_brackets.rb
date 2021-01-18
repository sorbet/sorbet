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

  # Re-assigning a literal type doesn't need to be pinned, by analogy with normal pinned variables.
  opts = {
    foo: 'hello'
  }
  T.reveal_type(opts) # error: Revealed type: `{foo: String("hello")} (shape of T::Hash[T.untyped, T.untyped])`
  opts[:foo] = 'world'
  # This type is wrong, because the key `foo:` is now `String("world")`, but that's no worse than normal pinned variables
  T.reveal_type(opts) # error: Revealed type: `{foo: String("hello")} (shape of T::Hash[T.untyped, T.untyped])`
end

sig {params(x: {foo: Integer, 'bar' => Float}).void}
def test3(x)
  x[:foo] = '' # error: Expected `Integer` but found `String("")` for key `Symbol(:foo)`
  T.reveal_type(x[:foo]) # error: Revealed type: `T.untyped`

  # This won't error until we change the `underlying` on Shape types to be stricter
  x[true] = nil
end
