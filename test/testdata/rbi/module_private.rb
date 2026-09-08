# typed: true

class A
  x = private :foo
  T.reveal_type(x) # error: `Symbol`
  x = private 'foo'
  T.reveal_type(x) # error: `String`
  x = private :foo, :bar
  T.reveal_type(x) # error: `T::Array[T.any(Symbol, String)]`
  x = private :foo, 'bar'
  T.reveal_type(x) # error: `T::Array[T.any(Symbol, String)]`
  x = private [:foo, :bar]
  #           ^^^^^^^^^^^^ error: Expected `Symbol` but found `[Symbol(:foo), Symbol(:bar)]` for argument `method_name`
  T.reveal_type(x) # error: `Symbol`
  x = private [:foo, 'bar']
  #           ^^^^^^^^^^^^^ error: Expected `Symbol` but found `[Symbol(:foo), String("bar")]` for argument `method_name`
  T.reveal_type(x) # error: `Symbol`

  x = private [:foo], :bar
  #           ^^^^^^ error: Expected `T.any(Symbol, String)` but found `[Symbol(:foo)]` for argument `method_name`
  x = private [:foo], [:bar]
  #           ^^^^^^ error: Expected `T.any(Symbol, String)` but found `[Symbol(:foo)]` for argument `method_name`
  #                   ^^^^^^ error: Expected `T.any(Symbol, String)` but found `[Symbol(:bar)]` for argument `rest`
end
