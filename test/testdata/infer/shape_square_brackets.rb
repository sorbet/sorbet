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

sig {params(x: {foo: Integer}).void}
def test2(x)
  x[:foo] = 1
  x[:foo] = '' # error: Expected `Integer` but found `String("")` for argument `arg1`
          # ^^ error: Expected `Integer` but found `String("")` for key `Symbol(:foo)`

  # Likewise, neither of these report errors, for backwards compatibility
  # We should work to make this stricter.
  x[:bar] = 1
  x[:bar] = ''

  # Re-assigning a literal type doesn't need to be pinned, by analogy with normal pinned variables.
  opts = {
    foo: 'hello'
  }
  T.reveal_type(opts) # error: Revealed type: `{foo: String("hello")} (shape of T::Hash[Symbol, String])`
  opts[:foo] = 'world'
  # This type is wrong, because the key `foo:` is now `String("world")`, but that's no worse than normal pinned variables
  T.reveal_type(opts) # error: Revealed type: `{foo: String("hello")} (shape of T::Hash[Symbol, String])`

  # Non-literal type for key
  opts[String.new] = 'good_value' # error: Expected `Symbol` but found `String` for argument `arg0`
end

sig {params(x: {foo: Integer, 'bar' => Float}).void}
def test3(x)
  T.reveal_type(x) # error: Revealed type: `{foo: Integer, String("bar") => Float} (shape of T::Hash[T.any(Symbol, String), T.any(Integer, Float)])`
  x[:foo] = '' # error: Expected `Integer` but found `String("")` for key `Symbol(:foo)`
          # ^^ error: Expected `T.any(Integer, Float)` but found `String("")` for argument `arg1`
  T.reveal_type(x[:foo]) # error: Revealed type: `Integer`

  x[true] = 1.0 # error: Expected `T.any(Symbol, String)` but found `TrueClass` for argument `arg0`
  x['bar'] = nil # error: Expected `Float` but found `NilClass` for key `String("bar")`
           # ^^^ error: Expected `T.any(Integer, Float)` but found `NilClass` for argument `arg1`
end
